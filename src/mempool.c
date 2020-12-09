/** @file       mempool.c
 *  @brief      Lock free memory pool implementation.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-12 newly created.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include "packedptr.h"
#include "mempool.h"

struct Fragment {
    struct MemoryNode next;
    uint8_t data[];
};

#define FRAGMENT_MAKER() \
    (struct Fragment){   \
        .next = {        \
            .frag = 0,   \
            .count = 0,  \
        },               \
    }

#define MEMORY_POOL_MAKER(p, b, c) \
    (struct MemoryPool){           \
        .pool = (p),               \
        .val_bytes = (b),         \
        .capacity = (c),           \
        .freeable = 0,             \
        .head = {                  \
            .frag = 0,             \
            .count = 0,            \
        },                         \
    }

#define max(a, b) (((a) > (b)) ? (a) : (b))

static inline bool Equals(struct MemoryNode a, struct MemoryNode b)
{
    return (a.frag == b.frag) && (a.count == b.count);
}

static inline size_t AlignedValueBytes(size_t val_bytes)
{
    size_t frag_bytes = max(val_bytes, sizeof(struct Fragment));
    if (frag_bytes % 8) {
        frag_bytes += 8 - (frag_bytes % 8);
    }
    return frag_bytes;
}

static inline void PutFragment(struct MemoryPool *self, struct Fragment *frag)
{
    assert(((uintptr_t)frag & 0x3) == 0);

    struct MemoryNode next, orig = atomic_load(&self->head);
    uint32_t packed = PackPointer(self->pool, frag);
    do {
        frag->next.frag = orig.frag;
        next.frag = packed;
        next.count = orig.count + 1;
    } while (!atomic_compare_exchange_weak(&self->head, &orig, next));
    atomic_fetch_add(&self->freeable, 1);
}

static inline struct Fragment *PickFragment(struct MemoryPool *self)
{
    struct MemoryNode next, orig = atomic_load(&self->head);
    do {
        if (orig.frag == 0) {
            errno = ENOMEM;
            return NULL;
        }
        next.frag = ((struct Fragment *)UnpackPointer(self->pool, orig.frag))->next.frag;
        next.count = orig.count + 1;
    } while (!atomic_compare_exchange_weak(&self->head, &orig, next));
    atomic_fetch_sub(&self->freeable, 1);

    return UnpackPointer(self->pool, orig.frag);
}

static inline void Setup(struct MemoryPool *self, void *pool, size_t val_bytes, size_t capacity)
{
    *self = MEMORY_POOL_MAKER(pool, val_bytes, capacity);
    size_t frag_bytes = AlignedValueBytes(val_bytes);
    for (size_t i = 0; i < self->capacity; i += 1) {
        struct Fragment *frag = (struct Fragment *)((uintptr_t)pool + (frag_bytes * i));
        *frag = FRAGMENT_MAKER();
        PutFragment(self, frag);
    }
}

ssize_t MemoryPool_ComputeSize(struct MemoryPool *self, size_t val_bytes, size_t capacity)
{
    if ((self == NULL) || (val_bytes == 0) || (capacity == 0)) {
        errno = EINVAL;
        return -1;
    }

    *self = MEMORY_POOL_MAKER(NULL, val_bytes, capacity);
    return AlignedValueBytes(val_bytes) * capacity;
}

int MemoryPool_Bind(struct MemoryPool *self, void *memory)
{
    if ((self == NULL) || (memory == NULL)) {
        errno = EINVAL;
        return -1;
    }

    Setup(self, memory, self->val_bytes, self->capacity);

    return 0;
}

int MemoryPool_Unbind(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    self->pool = NULL;

    return 0;
}

int MemoryPool_Clear(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    Setup(self, self->pool, self->val_bytes, self->capacity);

    return 0;
}

void *MemoryPool_Alloc(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return NULL;
    }

    return PickFragment(self);
}

void MemoryPool_Free(struct MemoryPool *self, void *ptr)
{
    if ((self == NULL) || (ptr == NULL)) {
        return;
    }

    PutFragment(self, ptr);
}

ssize_t MemoryPool_ValueBytes(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->val_bytes;
}

ssize_t MemoryPool_Capacity(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return self->capacity;
}

ssize_t MemoryPool_Freeable(struct MemoryPool *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return atomic_load(&self->freeable);
}

bool MemoryPool_Contains(struct MemoryPool *self, void *ptr)
{
    if (self == NULL) {
        errno = EINVAL;
        return false;
    }

    size_t frag_bytes = AlignedValueBytes(self->val_bytes);
    size_t pool_size = frag_bytes * self->capacity;
    void *pool_end = (void *)((uintptr_t)self->pool + pool_size);
    return (self->pool <= ptr) && (ptr < pool_end);
}
