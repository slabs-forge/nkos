/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 20/09/2011
 *
 * Variadic support
 */

#ifndef __KERNEL_STDARG_H__
#define __KERNEL_STDARG_H__

#include "kernel/types.h"

#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,t) __builtin_va_arg(v,t)

typedef __builtin_va_list va_list;

#endif

