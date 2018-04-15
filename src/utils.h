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

/** @addtogroup cat_utils ユーティリティ
 *  便利な機能を提供するモジュール.
 *  @{
 */

/**
 *  文字列結合マクロ.
 */
#define cat(a, b) a ## b

/**
 *  マクロ展開対応の文字列結合マクロ.
 */
#define macro_cat(a, b) cat(a, b)

/**
 *  汎用排他取得マクロ.
 */
#define generic_lock(obj)                           \
    _Generic((obj),                                 \
             pthread_mutex_t *: pthread_mutex_lock, \
             pthread_spinlock_t *: pthread_spin_lock)(obj)

/**
 *  汎用排他解放マクロ.
 */
#define generic_unlock(obj)                           \
    _Generic((obj),                                   \
             pthread_mutex_t *: pthread_mutex_unlock, \
             pthread_spinlock_t *: pthread_spin_unlock)(obj)

/**
 *  キャンセル可能な汎用排他取得マクロ.
 */
#define generic_cancellable_lock(obj)                                          \
    _Generic((obj),                                                            \
             pthread_mutex_t *: pthread_mutex_lock)(obj);                      \
    pthread_cleanup_push(                                                      \
        (void (*)(void *))_Generic((obj),                                      \
                                   pthread_mutex_t *: pthread_mutex_unlock,    \
                                   pthread_spinlock_t *: pthread_spin_unlock), \
        (obj))

/**
 *  キャンセル可能な汎用排他解放マクロ.
 */
#define generic_cancellable_unlock(obj)                     \
    _Generic((obj),                                         \
             pthread_mutex_t *: pthread_mutex_unlock)(obj); \
    pthread_cleanup_pop(0)

/**
 *  同期ブロックマクロ.
 *
 *  同期オブジェクトを利用して, 続くブロックを排他制御する.
 *  複数の処理を行う場合は @ref lock を使用すること.
 *
 *  @warning    ブロック内で, return, break, continue, goto はしないこと.
 *
 *  @par        使用例
 *              @code
 *              pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 *              synchronized (&mutex) {
 *                  // anything to do.
 *              }
 *              @endcode
 */
#if !defined(__clang__)
#define synchronized(obj)                                             \
    void macro_cat(__caller__, __LINE__)(void (*fn)(void)) {          \
        generic_lock(obj);                                            \
        fn();                                                         \
        generic_unlock(obj);                                          \
    }                                                                 \
    auto void macro_cat(__callee__, __LINE__)(void);                  \
    macro_cat(__caller__, __LINE__)(macro_cat(__callee__, __LINE__)); \
    void macro_cat(__callee__, __LINE__)(void)
#else
#warning 'synchronized' is not supported in clang
#define synchronized(obj)
#endif

/**
 *  同期ブロックマクロ.
 *
 *  同期オブジェクトを利用して, 続くブロックを排他制御する.
 *  インクリメントのみの場合などは @ref synchronized を使用するほうが
 *  パフォーマンスが良い.
 *
 *  @warning    ブロック内で, return, break, continue, goto はしないこと.
 *
 *  @par        使用例
 *              @code
 *              pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 *              lock (&mutex) {
 *                  // anything to do.
 *              }
 *              @endcode
 */
#if !defined(__clang__)
#define lock(obj)                                                     \
    void macro_cat(__caller__, __LINE__)(void (*fn)(void)) {          \
        generic_cancellable_lock(obj);                                \
        fn();                                                         \
        generic_cancellable_unlock(obj);                              \
    }                                                                 \
    auto void macro_cat(__callee__, __LINE__)(void);                  \
    macro_cat(__caller__, __LINE__)(macro_cat(__callee__, __LINE__)); \
    void macro_cat(__callee__, __LINE__)(void)
#else
#warning 'lock' is not supported in clang
#define lock(obj)
#endif

/** @} */

/** @addtogroup cat_safe_queue 安全な Queue 構造
 *  スレッドセーフな Queue 構造を提供するモジュール.
 *  @ingroup cat_utils
 *  @{
 */

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

/** @} */

#endif /* __ANTTQ_UTILS_H__ */
