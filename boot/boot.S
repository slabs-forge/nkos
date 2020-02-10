/*
 * NK MBR
 * 
 * Author: slaurent
 */

#include "boot/boot.h"

.code16gcc

.global _start

.text
_start:
	/*
	 * Setup stack and data in segment 0x0000
	 */
	cli
	xor %ax,%ax
	mov %ax,%ss
	mov %ax,%ds
	mov %ax,%es
	mov $NKBL_STACK,%sp
	sti

	/*
 	 * Saving boot drive
         */
	movb %dl,drive

	mov $msg0,%si
	callw _print

	mov $dap, %si
	callw _load
	jc _error

	mov 4(%si),%di
	mov (%di),%eax
	dec %eax
	or %eax,%eax
	jz _succes
	
	add $0x200,%di
	mov %ax,2(%si)
	mov %di,4(%si)
	incl 8(%si)
	callw _load
	jc _error

_succes:
	mov $msg2,%si
	callw _print
	
	xor %edx,%edx
	mov drive,%dl

	mov $NKBL_LOADER_START,%eax
	jmp *%eax
_error:
	mov $msg1,%si
	callw _print

	/*
	 * Report the error to the BIOS (and continue the boot process)
	 */
	int $0x18
_stop:
	jmp _stop
/*
 * Print message in teletex mode
 */ 
_print:
	mov $0x0e, %ah
1:
	lodsb
	or %al,%al
	jz 2f
	int $0x10
	jmp 1b
2:
	ret

/*
 * Load sector with int $0x13 - 0x42
 */
_load:
	mov $0x42,%ah
	movb drive,%dl
	int $0x13
	ret
.data
/*
 * Disk Address Packet
 */
dap:
	.byte	0x10
	.byte	0x00
	.word 	0x0001		// Sector count
	.word	0x8000		// Offset
	.word   0x0000		// Segment
	.long 	0x00000001	// Sector position (Low part)
	.long   0x00000000	// Sector position (High part)

drive: 	.byte 0x80

/*
 * Text messages
 */
msg0:
	.asciz "NKLoader starting..."
msg1:
	.asciz "Failed\r\n"
msg2:
	.asciz "Done\r\n"
	.asciz "Test"
