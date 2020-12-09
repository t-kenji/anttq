/** @file   anttq.cpp
 *  @brief  AntTQ API のテスト.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2018-03-18 新規作成.
 */

#include <atomic>
#include <vector>
#include <catch2/catch.hpp>

#include "utils.hpp"

extern "C" {
#include "anttq.h"
}

#include <cstdio>

class BitFlags {
private:
    std::atomic<uint32_t> *flags_;

public:
    BitFlags(size_t width) {
        flags_ = new std::atomic<uint32_t>[(width / 32) + 1]{};
    }

    ~BitFlags() {
        delete[] flags_;
    }

    void Set(size_t bit) {
        flags_[bit >> 5] |= (1 << (bit & 31));
    }

    void Unset(size_t bit) {
        flags_[bit >> 5] &= ~(1 << (bit & 31));
    }

    bool Get(size_t bit) {
        return !!(flags_[bit >> 5].load() & (1 << (bit & 31)));
    }
};

SCENARIO("タスクキューが初期化できること", tags("taskq", "init")) {
    GIVEN("特になし") {
        WHEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
            struct TaskQueue *tq = AntTQ_Init(1, 1);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(tq != NULL);
            }

            AntTQ_Term(tq);
        }

        WHEN("タスクキューを容量 5, ワーカー 1 で初期化する") {
            struct TaskQueue *tq = AntTQ_Init(5, 1);

            THEN("インスタンスが NULL ではないこと") {
                REQUIRE(tq != NULL);
            }

            AntTQ_Term(tq);
        }
    }
}

SCENARIO("タスクが処理できること", tags("taskq", "run")) {
    GIVEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
        struct TaskQueue *tq = AntTQ_Init(1, 1);
        AntTQ_Start(tq);

        WHEN("タスクを 1 件追加する") {
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                *param = 0xAA;
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param{0x55};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.arg = &param;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスクが呼び出されること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param == 0xAA);
            }
        }

        AntTQ_Term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 1 で初期化する") {
        struct TaskQueue *tq = AntTQ_Init(5, 1);
        AntTQ_Start(tq);

        WHEN("タスクを 5 件追加する") {
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                *param -= 1;
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param1{0x11},
                param2{0x22},
                param3{0x33},
                param4{0x44},
                param5{0x55};

            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.arg = &param1;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

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

        AntTQ_Term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 3 で初期化する") {
        struct TaskQueue *tq = AntTQ_Init(5, 3);
        AntTQ_Start(tq);

        WHEN("タスクを 5 件追加する") {
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                *param -= 1;
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param1{0x11},
                param2{0x22},
                param3{0x33},
                param4{0x44},
                param5{0x55};

            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.arg = &param1;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

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

        AntTQ_Term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 5 で初期化する") {
        struct TaskQueue *tq = AntTQ_Init(5, 5);
        AntTQ_Start(tq);

        WHEN("タスクを 5 件追加する") {
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                *param -= 1;
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param1{0x11},
                param2{0x22},
                param3{0x33},
                param4{0x44},
                param5{0x55};

            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.arg = &param1;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

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

        AntTQ_Term(tq);
    }
}

SCENARIO("タスクの失敗時にリトライできること", tags("taskq", "run", "retry")) {
    GIVEN("タスクキューを容量 1, ワーカー 1 で初期化する") {
        struct TaskQueue *tq{AntTQ_Init(1, 1)};
        AntTQ_Start(tq);

        WHEN("失敗するタスクをリトライ 3 回で 1 件追加する") {
            static const int retry_count{3};
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                ++(*param);
                return false;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param{0};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.arg = &param;
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("3 回のリトライが行われること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param == retry_count + 1);
            }
        }

        AntTQ_Term(tq);
    }

    GIVEN("タスクキューを容量 5, ワーカー 3 で初期化する") {
        struct TaskQueue *tq{AntTQ_Init(5, 3)};
        AntTQ_Start(tq);

        WHEN("失敗するタスクをリトライ 3 回で 5 件追加する") {
            static const int retry_count{3};
            auto runner = [&](TaskId, void *arg) -> bool {
                int *param{(int *)arg};
                ++(*param);
                return false;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            int param1 = 0,
                param2 = 0,
                param3 = 0,
                param4 = 0,
                param5 = 0;
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.retry = retry_count;
            item.arg = &param1;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param2;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param3;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param4;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);
            item.arg = &param5;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("3 回のリトライが行われること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(param1== retry_count + 1);
                REQUIRE(param2== retry_count + 1);
                REQUIRE(param3== retry_count + 1);
                REQUIRE(param4== retry_count + 1);
                REQUIRE(param5== retry_count + 1);
            }
        }

        AntTQ_Term(tq);
    }
}

SCENARIO("タスク状態がコールバックで通知されること", tags("taskq", "run", "callback")) {
    GIVEN("タスクキューを容量 10, ワーカー 1 で初期化する") {
        struct TaskQueue *tq{AntTQ_Init(10, 1)};
        AntTQ_Start(tq);

        WHEN("成功するタスクを 1 件追加する") {
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return true;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスク状態のコールバックが適宜呼ばれること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == 1);
                REQUIRE(statuses.size() == 2);
                REQUIRE(statuses[0] == TS_ACK);
                REQUIRE(statuses[1] == TS_SUCCESS);
            }
        }

        WHEN("失敗するタスクをリトライ 3 回で 1 件追加する") {
            static const int retry_count{3};
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return false;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスク状態のコールバックが適宜呼ばれること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == retry_count + 1);
                REQUIRE(statuses.size() == 8);
                REQUIRE(statuses[0] == TS_ACK);
                REQUIRE(statuses[1] == TS_RETRY);
                REQUIRE(statuses[2] == TS_ACK);
                REQUIRE(statuses[3] == TS_RETRY);
                REQUIRE(statuses[4] == TS_ACK);
                REQUIRE(statuses[5] == TS_RETRY);
                REQUIRE(statuses[6] == TS_ACK);
                REQUIRE(statuses[7] == TS_FAIL);
            }
        }

        WHEN("ACK でキャンセルするタスクを 1 件追加する") {
            static const int retry_count{3};
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return true;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                if (status == TS_ACK) {
                    return false;
                }
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスクがキャンセルされること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == 0);
                REQUIRE(statuses.size() == 1);
                REQUIRE(statuses[0] == TS_ACK);
            }
        }

        WHEN("RETRY 1 回目でキャンセルするタスクを 1 件追加する") {
            static const int retry_count{3};
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return false;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                if ((status == TS_RETRY) && (statuses.size() == 2)) {
                    return false;
                }
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスクがキャンセルされること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == 1);
                REQUIRE(statuses.size() == 2);
                REQUIRE(statuses[0] == TS_ACK);
                REQUIRE(statuses[1] == TS_RETRY);
            }
        }

        WHEN("RETRY 2 回目でキャンセルするタスクを 1 件追加する") {
            static const int retry_count{3};
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return false;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                if ((status == TS_RETRY) && (statuses.size() == 4)) {
                    return false;
                }
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスクがキャンセルされること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == 2);
                REQUIRE(statuses.size() == 4);
                REQUIRE(statuses[0] == TS_ACK);
                REQUIRE(statuses[1] == TS_RETRY);
                REQUIRE(statuses[2] == TS_ACK);
                REQUIRE(statuses[3] == TS_RETRY);
            }
        }

        WHEN("RETRY 3 回目でキャンセルするタスクを 1 件追加する") {
            static const int retry_count{3};
            int task_called{0};
            std::vector<enum TaskStatus> statuses;
            auto runner = [&](TaskId, void *) -> bool {
                task_called += 1;
                return false;
            };
            auto callbackee = [&](TaskId, enum TaskStatus status, void *) -> bool {
                statuses.push_back(status);
                if ((status == TS_RETRY) && (statuses.size() == 6)) {
                    return false;
                }
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            item.Callback = Lambda::cify<bool, TaskId, enum TaskStatus, void *>(callbackee);
            item.retry = retry_count;
            REQUIRE(AntTQ_Enqueue(tq, &item) >= 0);

            THEN("タスクがキャンセルされること") {
                /* 非同期処理が終わるのを待つ. */
                msleep(100);

                REQUIRE(task_called == 3);
                REQUIRE(statuses.size() == 6);
                REQUIRE(statuses[0] == TS_ACK);
                REQUIRE(statuses[1] == TS_RETRY);
                REQUIRE(statuses[2] == TS_ACK);
                REQUIRE(statuses[3] == TS_RETRY);
                REQUIRE(statuses[4] == TS_ACK);
                REQUIRE(statuses[5] == TS_RETRY);
            }
        }

        AntTQ_Term(tq);
    }
}

SCENARIO("タスク識別子が正しく反映されていること", tags("taskq", "run", "id")) {
    GIVEN("タスクキューを容量 10, ワーカー 1 で初期化する") {
        struct TaskQueue *tq{AntTQ_Init(10, 1)};
        AntTQ_Start(tq);

        WHEN("成功するタスクを 10 件追加する") {
            std::vector<TaskId> task_ids;
            auto runner = [&](TaskId id, void *) -> bool {
                task_ids.push_back(id);
                return true;
            };

            std::vector<TaskId> ids;
            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            for (int i = 0; i < 10; ++i) {
                ids.push_back(AntTQ_Enqueue(tq, &item));
            }

            /* 非同期処理が終わるのを待つ. */
            msleep(100);

            THEN("タスク識別子が呼び出し側, タスク側で一致すること") {
                auto i = GENERATE(Catch::Generators::range(1, 10));
                REQUIRE(task_ids[i] == ids[i]);
            }
        }

        AntTQ_Term(tq);
    }
}

SCENARIO("タスク削除できること", tags("taskq", "cancel")) {
    GIVEN("タスクキューを容量 10, ワーカー 1 で初期化する") {
        struct TaskQueue *tq{AntTQ_Init(10, 1)};
        AntTQ_Start(tq);

        WHEN("成功するタスクを 10 件追加し, 削除する") {
            std::vector<TaskId> task_ids;
            auto runner = [&](TaskId id, void *) -> bool {
                msleep(5);
                task_ids.push_back(id);
                return true;
            };

            std::vector<TaskId> ids;
            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            for (int i = 0; i < 10; ++i) {
                ids.push_back(AntTQ_Enqueue(tq, &item));
            }
            REQUIRE(AntTQ_Cancel(tq, ids[1]) == 0);
            ids.erase(ids.begin() + 1);
            REQUIRE(AntTQ_Cancel(tq, ids[2]) == 0);
            ids.erase(ids.begin() + 2);
            REQUIRE(AntTQ_Cancel(tq, ids[3]) == 0);
            ids.erase(ids.begin() + 3);
            REQUIRE(AntTQ_Cancel(tq, ids[4]) == 0);
            ids.erase(ids.begin() + 4);
            REQUIRE(AntTQ_Cancel(tq, ids[5]) == 0);
            ids.erase(ids.begin() + 5);

            /* 非同期処理が終わるのを待つ. */
            msleep(100);

            THEN("削除できること") {
                auto i = GENERATE(Catch::Generators::range(1, 5));
                REQUIRE(task_ids[i] == ids[i]);
            }
        }

        AntTQ_Term(tq);
    }
}

SCENARIO("連続動作確認", tags("taskq", "run")) {
    GIVEN("タスクキューを容量 30000, ワーカー 8 で初期化する") {
        static const size_t capacity{30000};
        static const size_t workers{8};
        struct TaskQueue *tq = AntTQ_Init(capacity, workers);
        REQUIRE(tq != NULL);
        AntTQ_Start(tq);

        WHEN("タスクを 100000 件追加する") {
            static const size_t width = 100000;
            class BitFlags flags(width);
            auto runner = [&](TaskId, void *arg) -> bool {
                size_t bit{(uintptr_t)arg};
                flags.Set(bit);
                return true;
            };

            struct TaskItem item{TASK_ITEM_INITIALIZER};
            item.Task = Lambda::cify<bool, TaskId, void *>(runner);
            for (size_t i = 0; i < width; i += 1) {
                item.arg = (void *)(uintptr_t)i;
                AntTQ_Enqueue(tq, &item);
            }

            /* 非同期処理が終わるのを待つ. */
            msleep(100);

            THEN("タスクがすべて呼び出されていること") {
                bool completed = true;
                for (size_t i = 0; i < width; i += 1) {
                    if (!flags.Get(i)) {
                        completed = false;
                        break;
                    }
                }
                REQUIRE(completed == true);
            }
        }

        AntTQ_Term(tq);
    }
}
