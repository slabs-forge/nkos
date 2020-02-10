/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 12/10/2011
 *
 * CPU info functions
 */

#ifndef __KERNEL_CPUINFO_H__
#define __KERNEL_CPUINFO_H__

#include "kernel/types.h"

#define CPU_CACHE_LEVEL_L1 	0
#define CPU_CACHE_LEVEL_L2 	1
#define CPU_CACHE_LEVEL_L3 	2
#define CPU_CACHE_LEVEL_MAX 	3

#define CPU_CACHE_TYPE_DATA	0
#define CPU_CACHE_TYPE_CODE	1
#define CPU_CACHE_TYPE_MAX	2

void cpuinfo_init(void);
 
uint64_t cpuinfo_get_cache_line(uint8_t,uint8_t);

#endif

