/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * Appel bios  
 */

#ifndef __KS_INTCALL_H__
#define __KS_INTCALL_H__

#include "kernel/boot/regs.h"

void intcall(uint8_t,struct regs_t*,struct regs_t*);

#endif

