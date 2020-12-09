/** @file       bitflag.h
 *  @brief      Bit flag implementation.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-20 newly created.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#ifndef __ANTTQ_BITFLAG_H__
#define __ANTTQ_BITFLAG_H__

#define bitflag(width)                      \
    struct {                                \
        _Atomic uint32_t array[((width) / 32) + 1]; \
    }

#define BITFLAG_INITIALIZER {}

#define BIT_TO_INDEX(x) ((x) >> 5)
#define BIT_TO_MASK(x) (1 << ((x) & 31))

#define bitflag_set(bf, bit)                               \
    do {                                                   \
        (bf).array[BIT_TO_INDEX(bit)] |= BIT_TO_MASK(bit); \
    } while (0)

#define bitflag_unset(bf, bit)                              \
    do {                                                    \
        (bf).array[BIT_TO_INDEX(bit)] &= ~BIT_TO_MASK(bit); \
    } while (0)

#define bitflag_get(bf, bit) !!((bf).array[BIT_TO_INDEX(bit)] & BIT_TO_MASK(bit))

#endif /* __ANTTQ_BITFLAG_H__ */
