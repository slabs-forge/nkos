/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * simple printf
 */

#ifndef __KERNEL_PRINTF_H__
#define __KERNEL_PRINTF_H__

struct out_t {
        void (*output)(int c);
        size_t (*room)(size_t size);
};

#include "kernel/stdarg.h"

int64_t kvprinto(struct out_t* out,const char* format, va_list ap);

#endif

