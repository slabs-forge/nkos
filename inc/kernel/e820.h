/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 29/09/2011
 *
 * e820 manipulation
 */

#ifndef __KERNEL_E820_H__
#define __KERNEL_E820_H__

#include "kernel/types.h"
#include "kernel/bootparm.h"

void e820_init(struct bootparm_t *);

#endif

