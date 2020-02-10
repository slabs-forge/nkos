/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * IO functions
 */

#ifndef __KERNEL_IO_H__
#define __KERNEL_IO_H__

#include "kernel/types.h"

static inline void outb(uint16_t port,uint8_t val) {
	asm("outb %0,%1"
		:
		: "a" (val) , "Nd" (port)
	);	
}

static inline uint8_t inb(uint16_t port) {
	uint8_t ret;

	asm("inb %1,%0"
		: "=a" (ret)
		: "Nd" (port)
	);

	return ret;
}

#endif

