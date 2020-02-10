/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * string function
 */

#ifndef __KERNEL_STRING_H__
#define __KERNEL_STRING_H__

#include "kernel/types.h"

void* memset(void* s,int c, size_t n);
void* memmove(void* dest,const void* src, size_t n);

size_t strlen(const char* s);

#endif

