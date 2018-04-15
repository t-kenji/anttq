/** @file   utils.c
 *  @brief  便利な機能を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-24 新規作成.
 *  @copyright  Copyright (c) 2018 t-kenji
 *
 *  This code is licensed under the MIT License.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE /* for pthread_spinlock_t */
#endif
#include <string.h>
#include <errno.h>

#include "utils.h"
#include "debug.h"

/**
 *  @details    キューを初期化し, 排他制御用のリソースを初期化する.
 *              削除を実現するため, 内部ではキューは @ref LIST で表現する.
 *
 *  @param      [out]   que             初期化対象のキューオブジェクト.
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        キューの容量.
 *  @param      [in]    compare         キュー操作用の比較関数.
 *  @return     成功時は, 0 が返り, @c que が初期化される.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は呼び出し側で保証すること.
 */
int safe_queue_init(struct safe_queue *que,
                    size_t payload_bytes,
                    size_t capacity,
                    COMPARATOR compare)
{
    *que = SAFE_QUEUE_INITIALIZER;
    que->list = list_init(payload_bytes, capacity);
    if (que->list == NULL) {
        return -1;
    }
    que->compare = compare;

    return 0;
}

/**
 *  @details    排他を行い, キューを解放する.
 *              @ref safe_queue_dequeue で待っているスレッドは再開させる.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @pre        @c que の非 NULL は呼び出し側で保証すること.
 */
void safe_queue_release(struct safe_queue *que)
{
    lock (&que->mutex) {
        list_release(que->list);
        que->list = NULL;
    }
    pthread_cond_broadcast(&que->inqueue);
}

/**
 *  @details    排他を行い, エンキューする.
 *              エンキュー後に条件変数を操作し, 通知を行う.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @param      [in]        payload 積むデータ.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は呼び出し側で保証すること.
 *  @pre        @c payload の非 NULL および, データ部以上のサイズを
 *              確保していることは呼び出し側で保証すること.
 */
int safe_queue_enqueue(struct safe_queue *que, void *payload)
{
    void *ret;

    lock (&que->mutex) {
        ret = list_insert(que->list, -1, payload);
    }
    pthread_cond_signal(&que->inqueue);

    return (ret != NULL) ? 0 : -1;
}

/**
 *  @details    キューからデータを取り出す.
 *              @c wait_for が true の場合は, キューが空の場合に条件変数を
 *              使用してエンキューされるのを待つ.
 *
 *  @param      [in,out]    que         キューオブジェクト.
 *  @param      [in]        wait_for    キューが空の場合は待つ.
 *  @param      [out]       payload データ部をコピーするバッファ.
 *  @return     成功時は, キューの残要素数が返り, @c payload にデータ部が
 *              コピーされる.
 *              キューが空 or 失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は呼び出し側で保証すること.
 *  @pre        @c payload の非 NULL および, データ部以上のサイズを
 *              確保していることは呼び出し側で保証すること.
 */
int safe_queue_dequeue(struct safe_queue *que, bool wait_for, void *payload)
{
    ssize_t payload_bytes = list_payload_bytes(que->list);
    int ret;

    if (payload_bytes < 1) {
        errno = EINVAL;
        return -1;
    }

    lock (&que->mutex) {
        if (wait_for) {
            while (list_count(que->list) == 0) {
                pthread_cond_wait(&que->inqueue, &que->mutex);
            }
        }
        ITER iter = list_iter(que->list);
        memcpy(payload, iter_get_payload(iter), payload_bytes);
        list_remove(que->list, iter);
        ret = list_count(que->list);
    }

    return ret;
}

/**
 *  @details    キューから指定のデータを削除する.
 *              @ref COMPARATOR が未設定の場合も, 失敗扱いとする.
 *
 *  @param      [in]    que     キューオブジェクト.
 *  @param      [in]    target  削除対象のデータ.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 */
int safe_queue_remove(struct safe_queue *que, const void *target)
{
    ITER iter;
    int ret;

    if (que->compare == NULL) {
        errno = ENOENT;
        return -1;
    }

    lock (&que->mutex) {
        for (iter = list_iter(que->list); iter != NULL; iter = iter_next(iter)) {
            if (que->compare(target, iter_get_payload(iter))) {
                break;
            }
        }
        if (iter == NULL) {
            errno = ENOENT;
            ret = -1;
        } else {
            list_remove(que->list, iter);
            ret = 0;
        }
    }

    return ret;
}

/**
 *  @details    現在のキューの内容を配列にコピーする.
 *
 *  @param      [in]    que     キューオブジェクト.
 *  @param      [out]   array   確保した配列のポインタ.
 *  @param      [out]   count   要素の数.
 *  @return     成功時は, 0 が返り, @c array に確保された配列のポインタを
 *              設定する.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は呼び出し側で保証すること.
 *  @pre        @c array の非 NULL は呼び出し側で保証すること.
 *  @pre        @c count の非 NULL は呼び出し側で保証すること.
 *  @warning    スレッドセーフではない.
 */
int safe_queue_to_array(struct safe_queue *que, void **array, size_t *count)
{
    int ret;

    lock (&que->mutex) {
        ret = list_to_array(que->list, array, count);
    }

    return ret;
}
