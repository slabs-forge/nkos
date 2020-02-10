/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 20/09/2011
 *
 * stdio emulation
 */

#ifndef __KS_UTL_STRING_H__
#define __KS_UTL_STRING_H__

#include "kernel/types.h"

void* memset(void* s,int c, size_t n);

size_t strlen(const char* s);

#endif

