/** @file   mempool.cpp
 *  @brief  メモリプールのテスト.
 *
 *  @author t-kenji <protect.2501@gmail.com>
 *  @date   2020-12-12 newly create.
 */

#include <catch2/catch.hpp>

#include "utils.hpp"

extern "C" {
#include "mempool.h"
}

SCENARIO("メモリプールに必要なメモリサイズが計算できること", tags("mempool")) {
    GIVEN("特になし") {
        WHEN("メモリプールを容量 0 で計算する") {
            MemoryPool mp;
            ssize_t pool_size = MemoryPool_ComputeSize(&mp, sizeof(int), 0);

            THEN("失敗すること") {
                REQUIRE(pool_size == -1);
            }
        }

        WHEN("メモリプールを容量 5 で計算する") {
            MemoryPool mp;
            size_t capacity{5};
            ssize_t pool_size = MemoryPool_ComputeSize(&mp, sizeof(int), capacity);

            THEN("計算できること") {
                REQUIRE((size_t)pool_size == 8 * capacity);
            }
        }

        WHEN("メモリプールを容量 512 で計算する") {
            MemoryPool mp;
            size_t capacity{512};
            ssize_t pool_size  = MemoryPool_ComputeSize(&mp, sizeof(int), capacity);

            THEN("計算できること") {
                REQUIRE((size_t)pool_size == 8 * capacity);
            }
        }
    }
}

SCENARIO("メモリプールからメモリを確保できること", tags("mempool")) {
    GIVEN("メモリプールを容量 5 で初期化しておく") {
        MemoryPool mp;
        size_t capacity{5};
        ssize_t pool_size = MemoryPool_ComputeSize(&mp, sizeof(int), capacity);
        REQUIRE(pool_size > 0);
        uint8_t *pool = new uint8_t[pool_size];
        REQUIRE(MemoryPool_Bind(&mp, pool) == 0);

        WHEN("メモリを確保しない") {
            THEN("メモリプールの空きが 5 であること") {
                REQUIRE(MemoryPool_Freeable(&mp) == 5);
            }
        }

        WHEN("メモリを 1 つ確保する") {
            REQUIRE(MemoryPool_Alloc(&mp) != NULL);

            THEN("メモリプールの空きが 4 であること") {
                REQUIRE(MemoryPool_Freeable(&mp) == 4);
            }
        }

        WHEN("メモリを 5 つ確保する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(MemoryPool_Alloc(&mp) != NULL);
            }

            THEN("メモリプールの空きが 0 であること") {
                REQUIRE(MemoryPool_Freeable(&mp) == 0);
            }
        }

        WHEN("メモリを 6 つ確保する") {
            for (int i = 0; i < 5; ++i) {
                REQUIRE(MemoryPool_Alloc(&mp) != NULL);
            }

            THEN("6 つ目の確保に失敗すること") {
                REQUIRE(MemoryPool_Alloc(&mp) == NULL);
                REQUIRE(MemoryPool_Freeable(&mp) == 0);
            }
        }

        MemoryPool_Unbind(&mp);
        delete[] pool;
    }
}

SCENARIO("メモリプールにメモリを解放できること", tags("mempool")) {
    GIVEN("メモリプールを容量 5 で初期化しておく") {
        MemoryPool mp;
        size_t capacity{5};
        ssize_t pool_size = MemoryPool_ComputeSize(&mp, sizeof(int), capacity);
        REQUIRE(pool_size > 0);
        uint8_t *pool = new uint8_t[pool_size];
        REQUIRE(MemoryPool_Bind(&mp, pool) == 0);

        WHEN("メモリを 1 つ確保した後に解放する") {
            int *ptr = (int *)MemoryPool_Alloc(&mp);
            REQUIRE(ptr != NULL);
            REQUIRE(MemoryPool_Freeable(&mp) == 4);

            MemoryPool_Free(&mp, ptr);

            THEN("メモリプールの空きが 5 に戻ること") {
                REQUIRE(MemoryPool_Freeable(&mp) == 5);
            }
        }

        MemoryPool_Unbind(&mp);
        delete[] pool;
    }
}
