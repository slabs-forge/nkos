/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 24/09/2011
 *
 * VGA driver
 */

#include "kernel/boot/code16gcc.h"

#include "kernel/boot/boot.h"
#include "kernel/boot/vga.h"
#include "kernel/boot/intcall.h"
#include "kernel/boot/utl/stdio.h"
#include "kernel/boot/utl/string.h"

int32_t vga_init(void) {
	struct regs_t regs;

	memset(&regs,0,sizeof(regs));
	regs.ax = 0x0003;

	intcall(0x10,&regs,0);

	printf("VGA Init done.\r\n");

	return 0;
}

