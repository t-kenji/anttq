/** @file       log.c
 *  @brief      logging functions.
 *
 *  @author     t-kenji <protect.2501@gmail.com>
 *  @date       2020-12-12 new creation.
 *  @copyright  Copyright (c) 2020 t-kenji
 *
 *  This code is licensed under the MIT License.
 */

#include <stdio.h>
#include <stdarg.h>

#include "log.h"

int println(const char *format, ...)
{
    char buffer[256] = {};

    va_list ap;
    int ret;

    va_start(ap, format);
    ret = vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
    va_end(ap);
    if (ret > (int)sizeof(buffer) - 1) {
        ret = sizeof(buffer) - 1;
    }
    buffer[ret] = '\n';
    ret += 1;

    return (fputs(buffer, stdout) != EOF) ? ret : -1;
}
