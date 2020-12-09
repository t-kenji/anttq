/*  @file   async_job.c
 *  @brief  非同期ジョブをテーマにした実装例.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "anttq.h"

#define DEBUG(fmt, ...) do{fprintf(stderr, "%s:%d:%s " fmt "\n",__FILE__,__LINE__,__func__,##__VA_ARGS__);}while(0)
#define MAYBE_UNUSED __attribute__((unused))

/*
 *  非同期に実行するジョブ.
 *
 *  @param  [in]    id  タスク ID.
 *  @param  [in]    arg エンキュー時に渡した固有情報.
 *  @return 成功時は, true を返す.
 *          失敗時は, false を返す.
 */
static bool async_job(TaskId id MAYBE_UNUSED, void *arg)
{
    int data = *(int *)arg;
    DEBUG("%d: anything to do.", data);

    return true;
}

/*
 *  実行ログは以下のようになる.
 *  @code
 *  $ ./example/async_job
 *  async_job.c:27:async_job 1: anything to do.
 *  async_job.c:27:async_job 2: anything to do.
 *  @endcode
 */
int main(int argc MAYBE_UNUSED, char **argv MAYBE_UNUSED)
{
    struct TaskQueue *tq = AntTQ_Init(10, 5);
    AntTQ_Start(tq);

    int data1 = 1, data2 = 2;
    struct TaskItem item = TASK_ITEM_INITIALIZER;
    item.Task = async_job;
    item.arg = &data1;
    AntTQ_Enqueue(tq, &item);
    item.arg = &data2;
    AntTQ_Enqueue(tq, &item);

    AntTQ_Term(tq);
    return 0;
}
