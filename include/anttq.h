/** @file   anttq.h
 *  @brief  組込み向け Task Queue システム API.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-23 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
/** @example    async_job.c
 *  非同期ジョブをテーマにした実装例.
 */
#ifndef __ANTTQ_TASKQ_H__
#define __ANTTQ_TASKQ_H__

#include <stdbool.h>

struct task_queue;

/** @addtogroup cat_anttq Task Queue
 *  Task Queue を構成するモジュール.
 *  @{
 */

/**
 *  タスク処理状態列挙子.
 */
enum task_status {
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
typedef int task_t;

/**
 *  タスク要素構造体.
 *
 *  @c task は @c arg を引数にして実行される.
 *  @c task が false を返した場合は, 同一タスクが再度エンキューされる.
 */
struct task_item {
    bool (*task)(task_t id, void *arg); /**< タスクとして実行される関数. */
    bool (*callback)(task_t id, enum task_status status, void *arg);
                                        /**< タスクの状態変化コールバック. */
    void *arg;                          /**< タスクに渡される引数. */
    int retry;                          /**< タスク失敗時のリトライ回数. */
};

/**
 *  タスク要素構造体の初期化子.
 */
#define TASK_ITEM_INITIALIZER \
    (struct task_item){       \
        .task = NULL,         \
        .callback = NULL,     \
        .arg = NULL,          \
        .retry = 0            \
    }

/**
 *  Task Queue の初期化を行う.
 */
struct task_queue *anttq_init(size_t capacity, int workers);

/**
 *  Task Queue を破棄する.
 */
void anttq_term(struct task_queue *tq);

/**
 *  タスクを予約する.
 */
task_t anttq_enqueue(struct task_queue *tq, struct task_item *item);

/**
 *  タスクをキャンセルする.
 */
int anttq_delete(struct task_queue *tq, task_t id);

/**
 *  現在のタスクをダンプする.
 */
void anttq_show_tasks(struct task_queue *tq);

/** @} */

#endif /* __ANTTQ_TASKQ_H__ */
