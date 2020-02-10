/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * register  
 */

#ifndef __KS_REGS_H__
#define __KS_REGS_H__

#include "kernel/types.h"

#define EFLAGS_CARRY	0x00000001

struct regs_t {
	union {
		struct {
			uint32_t eax;
			uint32_t ebx;
			uint32_t ecx;
			uint32_t edx;
			uint32_t esi;
			uint32_t edi;
			uint32_t eflags;
		};
		struct {
			uint16_t ax,ax1;
			uint16_t bx,bx1;
			uint16_t cx,cx1;
			uint16_t dx,dx1;
			uint16_t si,si1;
			uint16_t di,di1;
		};
	};
};

#endif

