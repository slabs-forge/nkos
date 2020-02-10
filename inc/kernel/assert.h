/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * panic
 */

#ifndef __KERNEL_ASSERT_H__
#define __KERNEL_ASSERT_H__

#include "kernel/types.h"
#include "kernel/utl/kdebug.h"

#define ASSERT(x) {if (!(x)) assert(__FILE__,__LINE__);}
//#define ASSERT(x) {(x);}

void panic(void) __attribute__((noreturn));

static inline void assert(const char* file,int line) {
	kdebug("**********************************************************\n");
	kdebug("KERNEL PANIC\n");
	kdebug("assertion failed at %s,%d\n",file,line);
	kdebug("**********************************************************\n");
	panic();
}

#endif

