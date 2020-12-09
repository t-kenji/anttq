/** @file       packedptr.h
 *  @brief      Packed Pointer Implementation.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-20 newly create.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#ifndef __ANTTQ_PACKEDPTR_H__
#define __ANTTQ_PACKEDPTR_H__

static inline uint32_t PackPointer(void *top, void *ptr)
{
    uint32_t up_diff = ((uintptr_t)ptr >> 32) - ((uintptr_t)top >> 32);
    return (uint32_t)((uintptr_t)ptr & 0xFFFFFFFC) | up_diff;
}

static inline void *UnpackPointer(void *top, uint32_t packed)
{
    uint32_t up_ptr = ((uintptr_t)top >> 32) + (packed & 0x3);
    uint32_t low_ptr = packed & 0xFFFFFFFC;
    return (void *)(((uintptr_t)up_ptr << 32) | low_ptr);
}

#endif /* __ANTTQ_PACKEDPTR_H__ */
