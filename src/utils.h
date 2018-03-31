/** @file   utils.h
 *  @brief  便利な機能を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-24 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
#ifndef __ANTTQ_UTILS_H__
#define __ANTTQ_UTILS_H__

#include <stdbool.h>
#include <pthread.h>

#include "collections.h"

/**
 *  キューの比較関数型.
 */
typedef bool (*COMPARATOR)(const void *, const void *);

/**
 *  スレッドセーフなキュー構造体.
 */
struct safe_queue {
    LIST list;              /**< キュー本体 */
    COMPARATOR compare;     /**< キュー操作用の比較関数. */
    pthread_mutex_t mutex;  /**< アクセス制御用ミューテックス. */
    pthread_cond_t inqueue; /**< エンキュー操作通知用の条件変数. */
};

/**
 *  スレッドセーフなキュー構造体の初期化子.
 */
#define SAFE_QUEUE_INITIALIZER              \
    (struct safe_queue){                    \
        .list = NULL,                       \
        .compare = NULL,                    \
        .mutex = PTHREAD_MUTEX_INITIALIZER, \
        .inqueue = PTHREAD_COND_INITIALIZER \
    }

/**
 *  スレッドセーフなキューを初期化する.
 */
int safe_queue_init(struct safe_queue *que,
                    size_t payload_bytes,
                    size_t capacity,
                    COMPARATOR compare);

/**
 *  キューを解放する.
 */
void safe_queue_release(struct safe_queue *que);

/**
 *  エンキューする.
 */
int safe_queue_enqueue(struct safe_queue *que, void *payload);

/**
 *  デキューする.
 */
int safe_queue_dequeue(struct safe_queue *que, bool wait_for, void *payload);

/**
 *  削除する.
 */
int safe_queue_remove(struct safe_queue *que, const void *target);

/**
 *  現在のキューを配列にコピーして取得する.
 */
int safe_queue_to_array(struct safe_queue *que, void **array, size_t *count);

#endif /* __ANTTQ_UTILS_H__ */
