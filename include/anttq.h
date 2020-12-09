/** @file       anttq.h
 *  @brief      API for Task Queue System for Embedded.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2018-03-23 newly created.
 *  @copyright  Copyright (c) 2018-2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
/** @example    async_job.c
 *  非同期ジョブをテーマにした実装例.
 */

#ifndef __ANTTQ_TASKQUEUE_H__
#define __ANTTQ_TASKQUEUE_H__

struct TaskQueue;

/** @addtogroup cat_taskqueue Task Queue
 *  This module compose the Task Queue.
 *  @{
 */

/**
 *  タスク処理状態列挙子.
 */
enum TaskStatus {
    TS_ACK,     /**< タスク処理開始. */
    TS_SUCCESS, /**< タスク処理完了. */
    TS_FAIL,    /**< タスク処理失敗. */
    TS_RETRY,   /**< タスク処理リトライ実施. */
    TS_LENGTH   /**< タスク処理状態数. */
};

/**
 *  タスク識別子.
 *
 *  タスクの予約時に発行されるタスクの識別子.
 *  無効値は -1 とする.
 */
typedef int16_t TaskId;

/**
 *  タスク要素構造体.
 *
 *  @c Task は @c arg を引数にして実行される.
 *  @c Task が false を返した場合は, 同一タスクが再度エンキューされる.
 */
struct TaskItem {
    bool (*Task)(TaskId id, void *arg); /**< タスクとして実行される関数. */
    bool (*Callback)(TaskId id, enum TaskStatus status, void *arg);
                                        /**< タスクの状態変化コールバック. */
    void *arg;                          /**< タスクに渡される引数. */
    int retry;                          /**< タスク失敗時のリトライ回数. */
};

/**
 *  タスク要素構造体の初期化子.
 */
#define TASK_ITEM_INITIALIZER \
    (struct TaskItem){        \
        .Task = NULL,         \
        .Callback = NULL,     \
        .arg = NULL,          \
        .retry = 0            \
    }

/**
 *  Task Queue の初期化を行う.
 */
struct TaskQueue *AntTQ_Init(size_t capacity, size_t workers);

/**
 *  Task Queue を破棄する.
 */
void AntTQ_Term(struct TaskQueue *self);

int AntTQ_Start(struct TaskQueue *self);
int AntTQ_Stop(struct TaskQueue *self);

/**
 *  タスクを予約する.
 */
TaskId AntTQ_Enqueue(struct TaskQueue *self, struct TaskItem *item);

/**
 *  タスクをキャンセルする.
 */
int AntTQ_Cancel(struct TaskQueue *self, TaskId id);

/** @} */

#endif /* __ANTTQ_TASKQUEUE_H__ */

