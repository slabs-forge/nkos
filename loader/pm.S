/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * PM & Real switching
 */

.global switch_rm
.global switch_pm

.text

.code16gcc

switch_pm:
	/*
	 * Loading GDT
	 */
	lgdt gdtdesc

	cli

	mov %cr0,%eax
	or $1,%eax
	mov %eax,%cr0

	data32 ljmp $0x0008,$_in_pm

.code32
_in_pm:
	mov $0x0010,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	mov %ax,%ss
	ret

.code32

switch_rm:
	ljmp $0x0018,$_in_cs16
_in_cs16:
.code16
	mov %cr0,%eax
	and $0xFFFFFFFE,%eax
	mov %eax,%cr0

	ljmp $0x0000,$_in_rm
_in_rm:
.code16gcc
	xor %ax,%ax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
	mov %ax,%ss
	cli
	ret

.data
.code32
gdt:

	/*
	 *	.word Segment Limit 15:00
	 * 	.word Base Address 15:00
	 *	.byte Base Address 23:16
	 *	.byte (P/DPL/S/Type:4)
	 *	.byte (G/D-B/L/AVL/Segment Limit 19:16)
	 *	.byte Base Address 31:24
	 */

	/*
	 * gdt[0] NULL Entry
	 */
	.word 0x0000,0x0000
	.byte 0x00, 0x00, 0x00, 0x00

	/*
	 * gdt[1] 32 bit code segment (256M, base 0x00000000, Code ER)
	 */
	.word 0xFFFF,0x0000
	.byte 0x00, 0x9A, 0xC0, 0x00

	/*
	 * gdt[2] 32 bit data segment (256M, base 0x00000000, Data RW)
	 */
	.word 0xFFFF,0x0000
	.byte 0x00, 0x92, 0xC0, 0x00

	/*
	 * gdt[3] 16 bit code segment (256Mo, base 0x00000000, Code ER)
	 */
	.word 0xFFFF,0x0000
	.byte 0x00, 0x9A, 0x80, 0x00

	/*
	 * gdt[4] 16 bit data segment (256Mo, base 0x00000000, Data RW)
	 */
	.word 0xFFFF,0x0000
	.byte 0x00, 0x92, 0x80, 0x00

	/*
	 * Descripteur GDT
	 */
gdtdesc:
	.word	0x0028
	.long	gdt
