/** @file   queue.cpp
 *  @brief  キューのテスト.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2020-12-20 newly create.
 */

#include <catch2/catch.hpp>

#include "utils.hpp"

extern "C" {
#include "queue.h"
}

SCENARIO("キューに必要なメモリサイズが計算できること", tags("queue")) {
    GIVEN("特になし") {
        WHEN("必要なメモリサイズを計算する") {
            size_t capacity{1};
            struct Queue que;
            ssize_t pool_size = Queue_ComputeSize(&que, sizeof(int), capacity);

            THEN("成功すること") {
                REQUIRE(pool_size > 0);
            }
        }

        WHEN("必要なメモリサイズを計算する") {
            size_t capacity{10000};
            struct Queue que;
            ssize_t pool_size = Queue_ComputeSize(&que, sizeof(int), capacity);

            THEN("成功すること") {
                REQUIRE(pool_size > 0);
            }
        }
    }
}

SCENARIO("キューに値を追加できること", tags("queue")) {
    GIVEN("サイズの十分なキューを作成する") {
        size_t capacity{10};
        struct Queue que;
        ssize_t pool_size = Queue_ComputeSize(&que, sizeof(int), capacity);
        REQUIRE(pool_size > 0);
        uint8_t *pool = new uint8_t[pool_size];
        REQUIRE(Queue_Bind(&que, pool) == 0);

        WHEN("値を追加する") {
            int value{10};

            THEN("成功すること") {
                REQUIRE(Queue_Enqueue(&que, &value) == 0);
            }
        }

        Queue_Unbind(&que);
        delete[] pool;
    }
}

SCENARIO("キューから値を取得できること", tags("queue")) {
    GIVEN("キューに値を追加しておく") {
        size_t capacity{10};
        int value{10};
        struct Queue que;
        ssize_t pool_size = Queue_ComputeSize(&que, sizeof(int), capacity);
        REQUIRE(pool_size > 0);
        uint8_t *pool = new uint8_t[pool_size];
        REQUIRE(Queue_Bind(&que, pool) == 0);
        REQUIRE(Queue_Enqueue(&que, &value) == 0);

        WHEN("値を取得する") {
            THEN("成功すること") {
                int result{-1};
                REQUIRE(Queue_Dequeue(&que, &result) == 0);
                REQUIRE(result == value);
            }
        }

        Queue_Unbind(&que);
        delete[] pool;
    }
}
