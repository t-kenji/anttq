/** @file       log.h
 *  @brief      logging functions.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-12 new creation.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#ifndef __ANTTQ_LOG_H__
#define __ANTTQ_LOG_H__

int println(const char *format, ...);

#define dbg(format, ...)                                           \
    do {                                                           \
        println("%s:%d(%s) " format, __FILE__, __LINE__, __func__, \
                ##__VA_ARGS__);                                    \
    } while (0)

#endif /* __ANTTQ_LOG_H__ */
