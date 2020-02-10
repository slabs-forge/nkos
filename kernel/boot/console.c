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
#include "kernel/boot/console.h"
#include "kernel/boot/regs.h"
#include "kernel/boot/intcall.h"

#include "kernel/boot/utl/string.h"

/*
 * Put char with bios
 */
void ks_putchar(int c) {
	struct regs_t in;
	
	memset(&in,0,sizeof(in));

	in.ax = c & 0xFF;
	in.ax |= 0x0e00;

	in.bx = 0x0000;
	in.cx = 0x0001;
	intcall(0x10,&in,0);
}

/*
 * Put string with bios
 */
void ks_putstring(const char* p) {
	size_t i = 0;
	while (*p) {
		ks_putchar(*p++);
	}
}

/*
 * Put n char of a string with bios
 */
void ks_putnstring(const char* p, size_t n) { 
	for (size_t i = 0; i < n; i++) {
		ks_putchar(p[i]);
	}
}
