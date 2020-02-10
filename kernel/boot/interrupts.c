/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * detect memory layout
 */

#include "kernel/boot/code16gcc.h"

#include "kernel/types.h"
#include "kernel/boot/boot.h"

void interrupts_enable() {
	asm("sti");
}

void interrupts_disable() {
	asm("cli");
}

