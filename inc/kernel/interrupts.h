/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 01/10/2011
 *
 * Interrupts related functions
 */

#ifndef __KERNEL_INTERRUPTS_H__
#define __KERNEL_INTERRUPTS_H__

static inline void lidt(void* p,uint16_t s) {
	struct {
		uint16_t size;
		uint64_t base;
	} __attribute__((packed,aligned(4))) _l;

	_l.size = s;
	_l.base = (uint64_t) p;

	asm( "lidt %0"
		: /* No output */
		: "m" (_l)
	);
}

#endif

