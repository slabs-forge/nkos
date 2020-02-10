/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * Amorce - main in real mode
 */

#include "kernel/boot/code16gcc.h"
#include "kernel/boot/boot.h"
#include "kernel/boot/console.h"
#include "kernel/boot/memory.h"
#include "kernel/boot/cpuid.h"
#include "kernel/boot/vbe.h"
#include "kernel/boot/vga.h"
#include "kernel/boot/gdt.h"
#include "kernel/boot/interrupts.h"

#include "kernel/boot/utl/stdio.h"

gdt_t gdt[5];

void main(void) {
	struct cpuid_t cpu;

	struct bootparm_t *bp = get_bootparm();

	/*
	 * Do not do this for the moment
	 * We ll need to draw our chars 
	 * as we'll later cant call bios
	 */
	//vbe_init();
	vga_init();
	
	/*
	 * Memory detection
	 */
	mem_init();

	/*
	 * Check for 64 bit CPU
	 */
	if (cpuid_support() == 0) {
		printf("cpuid not supported\r\n");
		return;
	}

	cpuid(0x80000000,&cpu);
	if (cpu.val1 < 0x80000001) {
		printf("cpuid(0x80000001) not supported\r\n");
		return;
	}

	cpuid(0x80000001,&cpu);
	if ( (cpu.val4 >> 29) & 0x1 == 0) {
		printf("cpu is not 64 bit compatible\r\n");
		return;
	}

	printf("Kernel offset %08x %08x\r\n",(uint32_t)(bp->kernel_offset >> 32),(uint32_t)bp->kernel_offset);
	printf("Kernel size   %08x %08x\r\n",(uint32_t)(bp->kernel_size >> 32),(uint32_t)bp->kernel_size);

	/*
	 * Loading GDT
	 * 
	 * gdt[1] code segment 4G
	 * gdt[2] data segment 4G
	 */
	gdt[0] = 0;
	gdt[1] = gdt_segment(0x00000000,0xFFFFF,0x9a);
	gdt[2] = gdt_segment(0x00000000,0xFFFFF,0x92);
	gdt[3] = 0x00209a0000000000;
	gdt[4] = 0x0040920000000000;
	printf("%08x %08x\r\n",(uint32_t)(gdt[0] >> 32),(uint32_t)gdt[0]);
	printf("%08x %08x\r\n",(uint32_t)(gdt[1] >> 32),(uint32_t)gdt[1]);
	printf("%08x %08x\r\n",(uint32_t)(gdt[2] >> 32),(uint32_t)gdt[2]);

	load_gdt(gdt,5);

	interrupts_disable();

	switch_pm(get_bootparm());
}
