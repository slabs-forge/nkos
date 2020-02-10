/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * debuging outpur
 */

#ifndef __KERNEL_KDEBUG_H__
#define __KERNEL_KEDBUG_H__

#include "kernel/stdarg.h"

int64_t kdebug(const char* format,...);

#endif

