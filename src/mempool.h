/** @file       mempool.h
 *  @brief      Lock free memory pool implementation.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-12 newly created.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#ifndef __ANTTQ_MEMPOOL_H__
#define __ANTTQ_MEMPOOL_H__

struct MemoryNode {
    uint32_t frag;
    uint32_t count;
};

struct MemoryPool {
    void *pool;
    size_t val_bytes;
    size_t capacity;
    size_t freeable;
    alignas(8) struct MemoryNode head;
};

#define MEMORY_POOL_INITIALIZER \
    {                           \
        .pool = NULL,           \
        .val_bytes = 0,        \
        .capacity = 0,          \
        .freeable = 0,          \
        .head = {               \
            .frag = 0,          \
            .count = 0,         \
        },                      \
    }

ssize_t MemoryPool_ComputeSize(struct MemoryPool *self, size_t val_bytes, size_t capacity);
int MemoryPool_Bind(struct MemoryPool *self, void *memory);
int MemoryPool_Unbind(struct MemoryPool *self);
int MemoryPool_Clear(struct MemoryPool *self);
void *MemoryPool_Alloc(struct MemoryPool *self);
void MemoryPool_Free(struct MemoryPool *self, void *ptr);
ssize_t MemoryPool_DataBytes(struct MemoryPool *self);
ssize_t MemoryPool_Capacity(struct MemoryPool *self);
ssize_t MemoryPool_Freeable(struct MemoryPool *self);
bool MemoryPool_Contains(struct MemoryPool *self, void *ptr);

#endif /* __ANTTQ_MEMPOOL_H__ */
