/** @file       taskqueue.c
 *  @brief      組込み向け Task Queue システム.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-19 新規作成.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <sched.h>
#include <errno.h>
#include <sys/types.h>
#include <pthread.h>

#include "utils.h"
#include "bitflag.h"
#include "queue.h"
#include "anttq.h"

/**
 *  作成する Worker の最大数.
 */
#define LIMIT_WORKERS (30)

/**
 *  Task Queue 管理構造体.
 */
struct TaskQueue {
    size_t num_of_workers;             /**< Worker の数. */
    uint32_t total_tasks;              /**< 予約されたタスクの総数. */
    pthread_t thrd_ids[LIMIT_WORKERS]; /**< Worker のスレッド ID 配列. */
    pthread_mutex_t mutex;
    pthread_cond_t inqueue;
    bool suspended;
    bitflag(INT16_MAX) canceled;
    struct Queue que;                  /**< タスクを保持するキュー. */
    uint8_t reserved[];
};

struct TaskItemCargo {
    TaskId id;            /**< タスク識別子. */
    struct TaskItem item; /**< タスク要素. */
};

/**
 *  何もしないタスク状態変化コールバック.
 *
 *  @param  [in]    id      タスク識別子.
 *  @param  [in]    status  タスク状態.
 *  @param  [in]    arg     タスク固有引数.
 *  @return true (処理継続) 固定.
 */
static bool NullCallback(TaskId id MAYBE_UNUSED, enum TaskStatus status MAYBE_UNUSED,
                         void *arg MAYBE_UNUSED)
{
    return true;
}

/**
 *  予約されたタスクの総数を更新する.
 *
 *  @param      [in,out]    self    Task Queue オブジェクト.
 *  @return     タスクの総数が返る.
 *  @pre        @c self の非 NULL は呼び出し側で保証する.
 *  @warning    変数がオーバーフローした場合は 0 に戻る.
 */
static uint32_t IncrementTotalTasks(struct TaskQueue *self)
{
    return __atomic_add_fetch(&self->total_tasks, 1, __ATOMIC_SEQ_CST);
}

/**
 *  タスク実行ワーカー.
 *
 *  キューからタスクを取り出し, 実行する.
 *  タスクが失敗した場合は, 指定に従いリトライを行う.
 *  @c callback が指定されており, かつ callback が false を返した場合は,
 *  処理を中断する.
 *
 *  @param  [in]    arg タスク固有引数.
 *  @pre    @c arg の非 NULL は呼び出し側で保証すること.
 */
static void *Worker(void *arg)
{
    struct TaskQueue *owner = (struct TaskQueue *)arg;

    while (true) {
        pthread_testcancel();

        struct TaskItemCargo cargo;
        lock (&owner->mutex) {
            while (atomic_load(&owner->suspended) || (Queue_Dequeue(&owner->que, &cargo) != 0)) {
                pthread_cond_wait(&owner->inqueue, &owner->mutex);
            }
        }

        do {
            TaskId id = cargo.id;
            struct TaskItem *item = &cargo.item;

            if (bitflag_get(owner->canceled, id)) {
                continue;
            }

            if (!item->Callback(id, TS_ACK, item->arg)) {
                continue;
            }
            bool result = item->Task(id, item->arg);
            if (!result && (item->retry > 0)) {
                if (!item->Callback(id, TS_RETRY, item->arg)) {
                    continue;
                }
                item->retry -= 1;
                if (Queue_Enqueue(&owner->que, &cargo) != 0) {
                    item->Callback(id, TS_FAIL, item->arg);
                }
            } else {
                item->Callback(id, (result ? TS_SUCCESS : TS_FAIL), item->arg);
            }
        } while (Queue_Dequeue(&owner->que, &cargo) == 0);
    }

    return NULL;
}

/**
 *  @details    指定の容量, ワーカー数で Task Queue を生成する.
 *
 *  @param      [in]    capacity    キューの容量.
 *  @param      [in]    workers     ワーカー数.
 *  @return     成功時は, 確保および初期化したオブジェクトのポインタを返る.
 *              失敗時は, NULL が返り, errno が適切に設定される.
 */
struct TaskQueue *AntTQ_Init(size_t capacity, size_t workers)
{
    if ((capacity == 0) || (workers == 0) || (INT16_MAX < capacity) || (LIMIT_WORKERS < workers)) {
        errno = EINVAL;
        return NULL;
    }

    struct Queue que;
    ssize_t pool_size = Queue_ComputeSize(&que, sizeof(struct TaskItemCargo), capacity);
    if (pool_size < 0) {
        return NULL;
    }

    struct TaskQueue *self = (struct TaskQueue *)malloc(sizeof(*self) + pool_size);
    if (self == NULL) {
        return NULL;
    }
    *self = (struct TaskQueue){
        .num_of_workers = workers,
        .total_tasks = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .inqueue = PTHREAD_COND_INITIALIZER,
        .suspended = true,
        .canceled = BITFLAG_INITIALIZER,
        .que = que,
    };
    Queue_Bind(&self->que, self->reserved);

    for (size_t i = 0; i < self->num_of_workers; i += 1) {
        int ret = pthread_create(&self->thrd_ids[i], NULL, Worker, self);
        if (ret != 0) {
            for (int j = i; j >= 0; j -= 1) {
                pthread_cancel(self->thrd_ids[j]);
            }
            for (int j = i; j >= 0; j -= 1) {
                pthread_join(self->thrd_ids[j], NULL);
            }
            free(self);
            errno = ret;
            return NULL;
        }
    }

    return self;
}

/**
 *  @details    @c self を解放する.
 *              @c self は AntTQ_Init() の戻り値である必要がある.
 *
 *  @param      [in,out]    self  Task Queue オブジェクト.
 */
void AntTQ_Term(struct TaskQueue *self)
{
    if (self != NULL) {
        for (size_t i = 0; i < self->num_of_workers; i += 1) {
            pthread_cancel(self->thrd_ids[i]);
        }
        for (size_t i = 0; i < self->num_of_workers; i += 1) {
            pthread_join(self->thrd_ids[i], NULL);
        }
        free(self);
    }
}

int AntTQ_Start(struct TaskQueue *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    atomic_store(&self->suspended,  false);
    lock (&self->mutex) {
        pthread_cond_broadcast(&self->inqueue);
    }

    return 0;
}

int AntTQ_Stop(struct TaskQueue *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    atomic_store(&self->suspended, true);

    return 0;
}

/**
 *  @details    指定のタスクを実行予約する.
 *
 *  @param      [in,out]    self    Task Queue オブジェクト.
 *  @param      [in]        item    予約するタスク情報.
 *  @return     成功時は, 予約したタスクの識別子が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 */
TaskId AntTQ_Enqueue(struct TaskQueue *self, struct TaskItem *item)
{
    if ((self == NULL) || (item == NULL) || (item->Task == NULL)) {
        errno = EINVAL;
        return -1;
    }

    /* ワーカーの処理をシンプルにするため, コールバックが設定されていない場合は
     * ダミーのコールバックを設定する.
     */
    if (item->Callback == NULL) {
        item->Callback = NullCallback;
    }

    struct TaskItemCargo cargo = {
        .id = IncrementTotalTasks(self) & INT16_MAX,
        .item = *item,
    };
    bitflag_unset(self->canceled, cargo.id);
    if (Queue_Enqueue(&self->que, &cargo) != 0) {
        return -1;
    }
    lock (&self->mutex) {
        pthread_cond_signal(&self->inqueue);
    }

    /* ワーカーのスループットを良くするため, CPU を明け渡す. */
    sched_yield();

    return cargo.id;
}

/**
 *  @details    @c id のタスクをキューから削除する.
 *              @c id がすでにキューから取り出されている場合は削除できない.
 *
 *  @param      [in,out]    self    Task Queue オブジェクト.
 *  @param      [in]        id  削除対象のタスク識別子.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 */
int AntTQ_Cancel(struct TaskQueue *self, TaskId id)
{
    if ((self == NULL) || (id < 0)) {
        errno = EINVAL;
        return -1;
    }

    bitflag_set(self->canceled, id);

    return 0;
}
