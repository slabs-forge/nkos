/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * cpuid support  
 */

#ifndef __KS_CPUID_H__
#define __KS_CPUID_H__

#include "kernel/types.h"

struct cpuid_t {
	uint32_t val1;	
	uint32_t val2;	
	uint32_t val3;	
	uint32_t val4;	
};

int32_t cpuid_support();
int32_t cpuid(uint32_t in,struct cpuid_t* out);

#endif

