/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 12/10/2011
 *
 * cpuid support  
 */

#ifndef __KERNEL_UTL_CPUID_H__
#define __KERNEL_UTL_CPUID_H__

#include "kernel/types.h"

struct cpuid_t {
	uint32_t eax;	
	uint32_t ebx;	
	uint32_t ecx;	
	uint32_t edx;	
};

void cpuid(struct cpuid_t* out);

#endif

