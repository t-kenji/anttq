/*  @file   async_job.c
 *  @brief  非同期ジョブをテーマにした実装例.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "anttq.h"

#define DEBUG(fmt, ...) do{fprintf(stderr, "%s:%d:%s " fmt "\n",__FILE__,__LINE__,__func__,##__VA_ARGS__);}while(0)

/*
 *  非同期に実行するジョブ.
 *
 *  @param  [in]    id  タスク ID.
 *  @param  [in]    arg エンキュー時に渡した固有情報.
 *  @return 成功時は, true を返す.
 *          失敗時は, false を返す.
 */
static bool async_job(task_t id, void *arg)
{
    int data = *(int *)arg;
    DEBUG("%d: anything to do.", data);

    return true;
}

/*
 *  実行ログは以下のようになる.
 *  @code
 *  $ ./example/async_job 
 *  async_job.c:24:async_job 1: anything to do.
 *  async_job.c:24:async_job 2: anything to do.
 *  @endcode
 */
int main(int argc, char **argv)
{
    int data1 = 1, data2 = 2;
    struct task_queue *tq = anttq_init(10, 5);
    struct task_item item = TASK_ITEM_INITIALIZER;
    item.task = async_job;
    item.arg = &data1;
    anttq_enqueue(tq, &item);
    item.arg = &data2;
    anttq_enqueue(tq, &item);

    anttq_term(tq);
    return 0;
}
