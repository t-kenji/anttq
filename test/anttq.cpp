/** @file   anttq.cpp
 *  @brief  AntTQ API のテスト.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 */
#include <ctime>
#include <unistd.h>
#include <catch.hpp>

extern "C" {
#include "debug.h"
#include "anttq.h"
}

int msleep(int msec)
{
    struct timespec req, rem = {msec / 1000, (msec % 1000) * 1000000};
    int ret;

    do {
        req = rem;
        ret = nanosleep(&req, &rem);
    } while ((ret == -1) && (errno == EINTR));

    return ret;
}

SCENARIO("タスクキューが初期化できること", "[taskq][init]") {
    GIVEN("特になし") {
        WHEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
            struct task_queue *tq = anttq_init(1, 1);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(tq != NULL);
            }

            anttq_term(tq);
        }

        WHEN("タスクキューを容量 5, ワーカー 1 で初期化する") {
            struct task_queue *tq = anttq_init(5, 1);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(tq != NULL);
            }

            anttq_term(tq);
        }
    }
}

SCENARIO("タスクが処理できること", "[taskq][run]") {
    GIVEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
        struct task_queue *tq = anttq_init(1, 1);

        WHEN("タスクを 1 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    *param = 0xAA;
                    return true;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param = 0x55;
            item.task = task::run;
            item.arg = &param;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("タスクが呼び出されること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param == 0xAA);
            }
        }

        anttq_term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 1 で初期化する") {
        struct task_queue *tq = anttq_init(5, 1);

        WHEN("タスクを 5 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    *param -= 1;
                    return true;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param1 = 0x11,
                param2 = 0x22,
                param3 = 0x33,
                param4 = 0x44,
                param5 = 0x55;

            item.task = task::run;
            item.arg = &param1;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("タスクが呼び出されること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param1 == 0x10);
                REQUIRE(param2 == 0x21);
                REQUIRE(param3 == 0x32);
                REQUIRE(param4 == 0x43);
                REQUIRE(param5 == 0x54);
            }

        }

        anttq_term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 3 で初期化する") {
        struct task_queue *tq = anttq_init(5, 3);

        WHEN("タスクを 5 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    *param -= 1;
                    return true;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param1 = 0x11,
                param2 = 0x22,
                param3 = 0x33,
                param4 = 0x44,
                param5 = 0x55;

            item.task = task::run;
            item.arg = &param1;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("タスクが呼び出されること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param1 == 0x10);
                REQUIRE(param2 == 0x21);
                REQUIRE(param3 == 0x32);
                REQUIRE(param4 == 0x43);
                REQUIRE(param5 == 0x54);
            }

        }

        anttq_term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 5 で初期化する") {
        struct task_queue *tq = anttq_init(5, 5);

        WHEN("タスクを 5 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    *param -= 1;
                    return true;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param1 = 0x11,
                param2 = 0x22,
                param3 = 0x33,
                param4 = 0x44,
                param5 = 0x55;

            item.task = task::run;
            item.arg = &param1;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("タスクが呼び出されること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param1 == 0x10);
                REQUIRE(param2 == 0x21);
                REQUIRE(param3 == 0x32);
                REQUIRE(param4 == 0x43);
                REQUIRE(param5 == 0x54);
            }

        }

        anttq_term(tq);
    }
}

SCENARIO("タスクの失敗時にリトライできること", "[taskq][run][retry]") {
    GIVEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
        struct task_queue *tq = anttq_init(1, 1);

        WHEN("失敗するタスクをリトライ 3 回で 1 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    ++(*param);
                    return false;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param = 0;
            item.task = task::run;
            item.arg = &param;
            item.retry = 3;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("3 回のリトライが行われること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param == 4);
            }
        }

        anttq_term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 3 で初期化する") {
        struct task_queue *tq = anttq_init(5, 3);

        WHEN("失敗するタスクをリトライ 3 回で 5 件追加する") {
            class task {
                public: static bool run(task_t id, void *arg) {
                    int *param = (int *)arg;
                    ++(*param);
                    return false;
                }
            };

            struct task_item item = TASK_ITEM_INITIALIZER;
            int param1 = 0,
                param2 = 0,
                param3 = 0,
                param4 = 0,
                param5 = 0;
            item.task = task::run;
            item.arg = &param1;
            item.retry = 3;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(anttq_enq(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(anttq_enq(tq, &item) >= 0);

            THEN("3 回のリトライが行われること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param1== 4);
                REQUIRE(param2== 4);
                REQUIRE(param3== 4);
                REQUIRE(param4== 4);
                REQUIRE(param5== 4);
            }
        }

        anttq_term(tq);
    }
}

SCENARIO("タスク状態がコールバックで通知されること", "[taskq][run][callback]") {
    GIVEN("タスクキューを容量 10, ワーカー 1 で初期化する") {
        struct task_queue *tq = anttq_init(10, 1);

        WHEN("成功するタスクを 1 件追加する") {
            struct param {
                task_t id;
                int data;
                int num_of_callbacks;
                enum task_status statuses[10];
            };
            class task {
                public: static bool run(task_t id, void *arg) {
                    struct param *param = (struct param *)arg;
                    param->id = id;
                    ++(param->data);
                    return true;
                }
                public: static void callback(task_t id, enum task_status status, void *arg) {
                    struct param *param = (struct param *)arg;
                    param->statuses[param->num_of_callbacks++] = status;
                }
            };

            task_t id;
            struct task_item item = TASK_ITEM_INITIALIZER;
            struct param param = {-1, 0, 0};
            item.task = task::run;
            item.callback = task::callback;
            item.arg = &param;
            id = anttq_enq(tq, &item);

            THEN("タスク状態のコールバックが適宜呼ばれること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param.id == id);
                REQUIRE(param.data == 1);
                REQUIRE(param.num_of_callbacks == 2);
                REQUIRE(param.statuses[0] == TS_ACK);
                REQUIRE(param.statuses[1] == TS_SUCCESS);
            }
        }

        WHEN("失敗するタスクをリトライ 3 回で 1 件追加する") {
            struct param {
                task_t id;
                int data;
                int num_of_callbacks;
                enum task_status statuses[10];
            };
            class task {
                public: static bool run(task_t id, void *arg) {
                    struct param *param = (struct param *)arg;
                    param->id = id;
                    ++(param->data);
                    return false;
                }
                public: static void callback(task_t id, enum task_status status, void *arg) {
                    struct param *param = (struct param *)arg;
                    param->statuses[param->num_of_callbacks++] = status;
                }
            };

            task_t id;
            struct task_item item = TASK_ITEM_INITIALIZER;
            struct param param = {-1, 0, 0};
            item.task = task::run;
            item.callback = task::callback;
            item.arg = &param;
            item.retry = 3;
            id = anttq_enq(tq, &item);

            THEN("タスク状態のコールバックが適宜呼ばれること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param.id == id);
                REQUIRE(param.data == 4);
                REQUIRE(param.num_of_callbacks == 8);
                REQUIRE(param.statuses[0] == TS_ACK);
                REQUIRE(param.statuses[1] == TS_RETRY);
                REQUIRE(param.statuses[2] == TS_ACK);
                REQUIRE(param.statuses[3] == TS_RETRY);
                REQUIRE(param.statuses[4] == TS_ACK);
                REQUIRE(param.statuses[5] == TS_RETRY);
                REQUIRE(param.statuses[6] == TS_ACK);
                REQUIRE(param.statuses[7] == TS_FAIL);
            }
        }

        anttq_term(tq);
    }
}


SCENARIO("タスク識別子が正しく反映されていること", "[taskq][run][id]") {
    GIVEN("タスクキューを容量 10, ワーカー 1 で初期化する") {
        struct task_queue *tq = anttq_init(10, 1);

        WHEN("成功するタスクを 10 件追加する") {
            struct param {
                task_t id;
            };
            class task {
                public: static bool run(task_t id, void *arg) {
                    struct param *param = (struct param *)arg;
                    param->id = id;
                    return true;
                }
            };

            task_t id[10];
            struct task_item item = TASK_ITEM_INITIALIZER;
            struct param param[10] = {-1};
            item.task = task::run;
            for (int i = 0; i < 10; ++i) {
                item.arg = &param[i];
                id[i] = anttq_enq(tq, &item);
            }
            anttq_show_tasks(tq);

            THEN("タスク状態のコールバックが適宜呼ばれること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                for (int i = 0; i < 10; ++i) {
                    REQUIRE(param[i].id == id[i]);
                }
            }
        }

        anttq_term(tq);
    }
}
