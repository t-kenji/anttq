/** @file   utils.h
 *  @brief  便利な機能を提供する.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-24 新規作成.
 */
#include "utils.h"
#include "debug.h"

/**
 *  @details    キューを初期化し, 排他制御用のリソースを初期化する.
 *
 *  @param      [out]   que             初期化対象のキューオブジェクト.
 *  @param      [in]    payload_bytes   データ部のサイズ.
 *  @param      [in]    capacity        キューの容量.
 *  @return     成功時は, 0 が返り, @c que が初期化される.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は上位で保証すること.
 */
int safe_queue_init(struct safe_queue *que,
                    size_t payload_bytes,
                    size_t capacity)
{
    *que = SAFE_QUEUE_INITIALIZER;
    que->que = queue_init(payload_bytes, capacity);
    if (que->que == NULL) {
        return -1;
    }
    return 0;
}

/**
 *  @details    排他を行い, キューを解放する.
 *              @ref safe_queue_deq で待っているスレッドは再開させる.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @pre        @c que の非 NULL は上位で保証すること.
 */
void safe_queue_release(struct safe_queue *que)
{
    pthread_mutex_lock(&que->mutex);
    queue_release(que->que);
    que->que = NULL;
    pthread_cond_broadcast(&que->inqueue);
    pthread_mutex_unlock(&que->mutex);
}

/**
 *  @details    排他を行い, エンキューする.
 *              エンキュー後に条件変数を操作し, 通知を行う.
 *
 *  @param      [in,out]    que     キューオブジェクト.
 *  @param      [in]        payload 積むデータ.
 *  @return     成功時は, 0 が返る.
 *              失敗時は, -1 が返り, errno が適切に設定される.
 *  @pre        @c que の非 NULL は上位で保証すること.
 *  @pre        @c payload の非 NULL および, データ部以上のサイズを
 *              確保していることは上位で保証すること.
 */
int safe_queue_enq(struct safe_queue *que, void *payload)
{
    void *ret;

    pthread_mutex_lock(&que->mutex);
    ret = queue_enq(que->que, payload);
    pthread_mutex_unlock(&que->mutex);
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
 *  @pre        @c que の非 NULL は上位で保証すること.
 *  @pre        @c payload の非 NULL および, データ部以上のサイズを
 *              確保していることは上位で保証すること.
 */
int safe_queue_deq(struct safe_queue *que, bool wait_for, void *payload)
{
    int ret;

    pthread_mutex_lock(&que->mutex);
    pthread_cleanup_push(pthread_mutex_unlock, &que->mutex);
    if (wait_for) {
        while (queue_count(que->que) == 0) {
            pthread_cond_wait(&que->inqueue, &que->mutex);
        }
    }
    ret = queue_deq(que->que, payload);
    pthread_mutex_unlock(&que->mutex);
    pthread_cleanup_pop(0);

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
 *  @warning    スレッドセーフではない.
 */
int safe_queue_to_array(struct safe_queue *que, void **array, size_t *count)
{
    int ret;

    pthread_mutex_lock(&que->mutex);
    pthread_cleanup_push(pthread_mutex_unlock, &que->mutex);
    ret = queue_to_array(que->que, array, count);
    pthread_mutex_unlock(&que->mutex);
    pthread_cleanup_pop(0);

    return ret;
}
