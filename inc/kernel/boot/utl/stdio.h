/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 20/09/2011
 *
 * stdio emulation
 */

#ifndef __KS_UTL_STDIO_H__
#define __KS_UTL_STDIO_H__

#include "kernel/stdarg.h"

int printf(const char* format,...);
int vprintf(const char* format, va_list ap);

#endif

