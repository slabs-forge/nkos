/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * Kernel start 
 */

#ifndef __KS_BOOT_H__
#define __KS_BOOT_H__

#include "kernel/types.h"
#include "kernel/bootparm.h"

static inline struct bootparm_t* get_bootparm() {
	return (struct bootparm_t*) 8;
}

extern void switch_pm(void*) __attribute__((noreturn));

#endif

