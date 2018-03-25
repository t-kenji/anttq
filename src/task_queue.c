/** @file   task_queue.c
 *  @brief  組込み向け Task Queue システム.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-23 新規作成.
 */
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <pthread.h>

#include "utils.h"
#include "debug.h"
#include "anttq.h"

/**
 *  作成する Worker の最大数.
 */
#define WORKER_MAX (30)

/**
 *  タスク要素の内部構造体.
 */
struct task_item_inner {
    task_t id;             /**< タスク識別子. */
    struct task_item item; /**< タスク要素. */
};

/**
 *  タスク要素の内部構造体の設定ヘルパ.
 */
#define TASK_ITEM_INNER_HELPER(id_, item_) \
    {                                      \
        .id = (id_),                       \
        .item = (item_)                    \
    }

/**
 *  タスク要素の内部構造体の初期化子.
 */
#define TASK_ITEM_INNER_INITIALIZER \
    (struct task_item_inner)TASK_ITEM_INNER_HELPER(-1, TASK_ITEM_INITIALIZER)

/**
 *  Task Queue 管理構造体.
 */
struct task_queue {
    int num_of_workers;             /**< Worker の数. */

    pthread_spinlock_t spinlock;    /**< 変数アクセス用の排他. */
    uint32_t total_tasks;           /**< 予約されたタスクの総数. */
    struct safe_queue que;          /**< タスクを保持するキュー. */
    pthread_t thrd_ids[WORKER_MAX]; /**< Worker のスレッド ID 配列. */
};

/**
 *  Task Queue 管理構造体の設定ヘルパ.
 */
#define TASK_QUEUE_HELPER(num)         \
    {                                  \
        .num_of_workers = (num),       \
        .total_tasks = 0,              \
        .que = SAFE_QUEUE_INITIALIZER  \
    }

/**
 *  Task Queue 管理構造体の初期化子.
 */
#define TASK_QUEUE_INITIALIZER(num) \
    (struct task_queue)TASK_QUEUE_HELPER((num))

/**
 *  何もしないタスク状態変化コールバック.
 *
 *  @param  [in]    id      タスク識別子.
 *  @param  [in]    status  タスク状態.
 *  @param  [in]    arg     タスク固有引数.
 */
static void null_callback(task_t id, enum task_status status, void *arg)
{
}

/**
 *  予約されたタスクの総数を更新する.
 *
 *  @param      [in,out]    tq  Task Queue オブジェクト.
 *  @return     タスクの総数が返る.
 *  @warning    変数がオーバーフローした場合は 0 に戻る.
 */
static uint32_t anttq_update_total_tasks(struct task_queue *tq)
{
    uint32_t num;
    pthread_spin_lock(&tq->spinlock);
    num = ++tq->total_tasks;
    pthread_spin_unlock(&tq->spinlock);
    return num;
}

/**
 *  Worker スレッド.
 *
 *  キューからタスクを取り出し, 実行する.
 *  タスクが失敗した場合は, 指定に従いリトライを行う.
 *
 *  @param  [in]    arg タスク固有引数.
 */
static void *anttq_worker(void *arg)
{
    struct safe_queue *que = (struct safe_queue *)arg;
    struct task_item_inner cardboard;
    struct task_item *item;
    bool result;
    int ret;

    while (true) {
        pthread_testcancel();

        cardboard = TASK_ITEM_INNER_INITIALIZER;
        safe_queue_deq(que, true, &cardboard);

        item = &cardboard.item;
        item->callback(cardboard.id, TS_ACK, item->arg);
        result = item->task(cardboard.id, item->arg);
        if (!result && (item->retry > 0)) {
            item->callback(cardboard.id, TS_RETRY, item->arg);
            --item->retry;
            ret = safe_queue_enq(que, &cardboard);
            if (ret != 0) {
                item->callback(cardboard.id, TS_FAIL, item->arg);
            }
        } else {
            item->callback(cardboard.id,
                           (result ? TS_SUCCESS : TS_FAIL),
                           item->arg);
        }
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
struct task_queue *anttq_init(size_t capacity, int workers)
{
    struct task_queue *tq;
    int ret;
    int i;

    if ((capacity <= 0) || (workers <= 0) || (WORKER_MAX < workers)) {
        errno = EINVAL;
        return NULL;
    }

    tq = (struct task_queue *)malloc(sizeof(*tq));
    if (tq == NULL) {
        return NULL;
    }
    *tq = TASK_QUEUE_INITIALIZER(workers);

    ret = pthread_spin_init(&tq->spinlock, 0);
    if (ret != 0) {
        errno = ret;
        anttq_term(tq);
        return NULL;
    }

    ret = safe_queue_init(&tq->que,
                          sizeof(struct task_item_inner),
                          capacity);
    if (ret != 0) {
        anttq_term(tq);
        return NULL;
    }
    for (i = 0; i < tq->num_of_workers; ++i) {
        ret = pthread_create(&tq->thrd_ids[i],
                             NULL,
                             anttq_worker,
                             &tq->que);
        if (ret != 0) {
            errno = ret;
            anttq_term(tq);
            return NULL;
        }
    }

    return tq;
}

/**
 *  @details    @c tq を解放する.
 *              @c tq は @ref anttq_init の戻り値である必要がある.
 *
 *  @param      [in,out]    tq  Task Queue オブジェクト.
 */
void anttq_term(struct task_queue *tq)
{
    if (tq != NULL) {
        int i;
        for (i = 0; i < tq->num_of_workers; ++i) {
            pthread_cancel(tq->thrd_ids[i]);
        }
        for (i = 0; i < tq->num_of_workers; ++i) {
            pthread_join(tq->thrd_ids[i], NULL);
        }
        safe_queue_release(&tq->que);
        pthread_spin_destroy(&tq->spinlock);
        free(tq);
    }
}

/**
 *  @details    指定のタスクを実行予約する.
 *
 *  @param      [in,out]    tq      Task Queue オブジェクト.
 *  @param      [in]        item    予約するタスク情報.
 *  @return     成功時は, 予約したタスクの識別子が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 */
task_t anttq_enq(struct task_queue *tq, struct task_item *item)
{
    struct task_item_inner cardboard;
    int ret;

    if ((tq == NULL) || (item == NULL) || (item->task == NULL)) {
        errno = EINVAL;
        return -1;
    }

    /* ワーカーの処理をシンプルにするため, コールバックが設定されていない場合は
     * ダミーのコールバックを設定する.
     */
    if (item->callback == NULL) {
        item->callback = null_callback;
    }

    cardboard.id = anttq_update_total_tasks(tq) % INT_MAX;
    cardboard.item = *item;
    ret = safe_queue_enq(&tq->que, &cardboard);
    if (ret != 0) {
        return -1;
    }

    /* ワーカーのスループットを良くするため, CPU を明け渡す. */
    sched_yield();

    return cardboard.id;
}

/**
 *  @details    予約中のタスクおよび, 実行中のタスクを標準出力に出す.
 *
 *  @param      [in]    tq  Task Queue オブジェクト.
 */
void anttq_show_tasks(struct task_queue *tq)
{
    struct task_item_inner *items;
    size_t count;
    int i;

    safe_queue_to_array(&tq->que, (void **)&items, &count);
    for (i = 0; i < count; ++i) {
        fprintf(stdout,
                "%3d) id: %d, task: %p, callback: %p, arg: %p, retry: %d\n",
                i + 1,
                items[i].id,
                items[i].item.task,
                items[i].item.callback,
                items[i].item.arg,
                items[i].item.retry);
    }
    free(items);
}
