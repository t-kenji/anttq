/** @file   utils.h
 *  @brief  便利な機能を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-24 新規作成.
 *  @copyright  Copyright (c) 2018-2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
#ifndef __ANTTQ_UTILS_H__
#define __ANTTQ_UTILS_H__

/** @addtogroup cat_utils ユーティリティ
 *  便利な機能を提供するモジュール.
 *  @{
 */

#define MAYBE_UNUSED __attribute__((unused))

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
#define generic_lock(obj) \
    _Generic((obj),       \
             pthread_mutex_t *: pthread_mutex_lock)(obj)

/**
 *  汎用排他解放マクロ.
 */
#define generic_unlock(obj) \
    _Generic((obj),         \
             pthread_mutex_t *: pthread_mutex_unlock)(obj)

/**
 *  キャンセル可能な汎用排他取得マクロ.
 */
#define generic_cancellable_lock(obj)                                                        \
    _Generic((obj),                                                                          \
             pthread_mutex_t *: pthread_mutex_lock)(obj);                                    \
    void macro_cat(__cleaner__, __LINE__)(void *macro_cat(__arg__, __LINE__) MAYBE_UNUSED) { \
        _Generic((obj),                                                                      \
                 pthread_mutex_t *: pthread_mutex_unlock)(obj);                              \
    }                                                                                        \
    pthread_cleanup_push(macro_cat(__cleaner__, __LINE__), (obj))

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

#endif /* __ANTTQ_UTILS_H__ */
