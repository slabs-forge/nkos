/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * kernel start
 */

.intel_syntax noprefix

.global _kstart
.global __text
.global __data
.global __bss
.global __end
.global panic

.code64
.section ".head"
	/*
	 * Jump to real start
	 */
	jmp _kstart
__text:	.quad _text
__data:	.quad _data
__bss:	.quad _bss
__end:  .quad _end

.text
_kstart:
	/*
	 *Reseting GDT in kernel adress space
	 */
	lgdt [gdt_desc]

	/*
	 * Reseting DS,ES,FS,GS,SS
	 */
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	mov ss,ax

	/*
	 * Reseting CS
	 */
	push 0x8
	lea rax,OFFSET 1f[rip]
	push rax
	lretq
.align 8
1:

	call kmain
panic:	hlt
	jmp panic

.data
gdt:
	.align 8

	/*
	 * GDT[0] Null segment
	 */
	.quad 0x0000000000000000

	/*
	 * GDT[1] Code Segment
	 */
	.quad 0x00209a0000000000

	/*
	 * GDT[2] Data Segment
	 */
	.quad 0x0040920000000000

	.align 16
gdt_desc:
	.word 0x17
	.quad gdt

