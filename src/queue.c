/** @file       queue.c
 *  @brief      Lock free queue implementation.
 *
 *              Simple, Fast, and Practical Non-Blocking and Blocking
 *              Concurrent Queue Algorithms
 *
 *              https://www.cs.rochester.edu/u/scott/papers/1996_PODC_queues.pdf
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
#include <errno.h>
#include <sys/types.h>

#include "mempool.h"
#include "packedptr.h"
#include "queue.h"

struct Node {
    struct Pointer next;
    uint8_t value[];
};

#define NodeMaker()     \
    (struct Node){      \
        .next = {       \
            .ptr = 0,   \
            .count = 0, \
        },              \
    }

static inline bool Equals(struct Pointer a, struct Pointer b)
{
    return (a.count == b.count) && (a.ptr == b.ptr);
}

static inline struct Node *AllocNode(struct Queue *self, const void *val)
{
    struct Node *node = MemoryPool_Alloc(&self->mp);
    if (node == NULL) {
        return NULL;
    }

    *node = NodeMaker();
    if (val != NULL) {
        memcpy(node->value, val, self->val_bytes);
    }

    return node;
}

ssize_t Queue_ComputeSize(struct Queue *self, size_t val_bytes, size_t capacity)
{
    size_t node_bytes = sizeof(struct Node) + val_bytes;
    struct MemoryPool mp;
    ssize_t pool_size = MemoryPool_ComputeSize(&mp, node_bytes, capacity + 1);
    if (pool_size < 0) {
        return -1;
    }

    self->mp = mp;
    self->val_bytes = val_bytes;
    return pool_size;
}

int Queue_Bind(struct Queue *self, void *memory)
{
    MemoryPool_Bind(&self->mp, memory);

    struct Node *node = AllocNode(self, NULL);
    struct Pointer ptr = {
        .ptr = PackPointer(self->mp.pool, node),
        .count = 0,
    };
    atomic_init(&self->head, ptr);
    atomic_init(&self->tail, ptr);

    return 0;
}

int Queue_Unbind(struct Queue *self)
{
    if (self == NULL) {
        errno = EINVAL;
        return -1;
    }

    return MemoryPool_Unbind(&self->mp);
}

int Queue_Enqueue(struct Queue *self, const void *val)
{
    if ((self == NULL) || (val == NULL)) {
        errno = EINVAL;
        return -1;
    }

    struct Node *node = AllocNode(self, val);
    if (node == NULL) {
        return -1;
    }

    void *top = self->mp.pool;
    struct Pointer tail, tmp;
    while (true) {
        tail = atomic_load(&self->tail);
        struct Node *ptr = (struct Node *)UnpackPointer(top, tail.ptr);
        struct Pointer next = ptr->next;

        if (Equals(tail, atomic_load(&self->tail))) {
            if (next.ptr == 0) {
                tmp.ptr = PackPointer(top, node);
                tmp.count = next.count + 1;
                if (atomic_compare_exchange_weak(&ptr->next, &next, tmp)) {
                    break;
                }
            } else {
                tmp.ptr = next.ptr;
                tmp.count = tail.count + 1;
                atomic_compare_exchange_weak(&self->tail, &tail, tmp);
            }
        }
    }
    tmp.ptr = PackPointer(top, node);
    tmp.count = tail.count + 1;
    atomic_compare_exchange_weak(&self->tail, &tail, tmp);

    return 0;
}

int Queue_Dequeue(struct Queue *self, void *val)
{
    if ((self == NULL) || (val == NULL)) {
        errno = EINVAL;
        return -1;
    }

    void *top = self->mp.pool;
    struct Pointer head;
    while (true) {
        head = atomic_load(&self->head);
        struct Pointer tail = atomic_load(&self->tail),
                       next = ((struct Node *)UnpackPointer(top, head.ptr))->next;

        if (Equals(head, atomic_load(&self->head))) {
            if (head.ptr == tail.ptr) {
                if (next.ptr == 0) {
                    errno = ENOENT;
                    return -1;
                }
                struct Pointer tmp = {
                    .ptr = next.ptr,
                    .count  = tail.count + 1,
                };
                atomic_compare_exchange_weak(&self->tail, &tail, tmp);
            } else {
                memcpy(val, ((struct Node *)UnpackPointer(top, next.ptr))->value, self->val_bytes);
                struct Pointer tmp = {
                    .ptr = next.ptr,
                    .count = head.count + 1,
                };
                if (atomic_compare_exchange_weak(&self->head, &head, tmp)) {
                    break;
                }
            }
        }
    }

    MemoryPool_Free(&self->mp, UnpackPointer(top, head.ptr));

    return 0;
}
