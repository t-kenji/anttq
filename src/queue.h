/** @file       queue.h
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

#ifndef __ANTTQ_QUEUE_H__
#define __ANTTQ_QUEUE_H__

#include "mempool.h"

struct Pointer {
    uint32_t ptr;
    uint32_t count;
};

struct Queue {
    struct MemoryPool mp;
    size_t val_bytes;
    alignas(8) struct Pointer head;
    alignas(8) struct Pointer tail;
};

ssize_t Queue_ComputeSize(struct Queue *self, size_t val_bytes, size_t capacity);
int Queue_Bind(struct Queue *self, void *memory);
int Queue_Unbind(struct Queue *self);
int Queue_Enqueue(struct Queue *self, const void *val);
int Queue_Dequeue(struct Queue *self, void *val);

#endif /* __ANTTQ_QUEUE_H__ */
