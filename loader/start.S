/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * start routine
 */

.global halt
.global jump

.code16gcc

.section ".head"
sect:	.long _sect

	/*
 	 *	Real code start
	 */ 
	jmp _start
	nop
	
	
	/*
	 * Boot parameters - start at $0x0004
	 */
_bp:
	/*
	 * Kernel image file
	 */
	.fill 128	
	
	/*
	 * Kernel address
	 */
load_addr:
	.long 0x00100000
jump_addr:
	.long 0x00100000

	/*
	 * End of boot parameters
	 */
.text
_start:
	/*
	 * Save drive reference
	 */
	mov %edx,drive_ref	

	/*
  	 * Switch on a20 line
	 */
	call a20_test
	or %eax,%eax
	jz 1f	
	call a20_enable
1:
	call switch_pm
.code32
	
	mov $_CTOR_LIST, %esi
	mov $_CTOR_SIZE, %ecx
1:	or %ecx,%ecx
	jz 2f
	push %esi
	push %ecx
	call *(%esi)
	pop %ecx
	pop %esi
	addl $4,%esi
	dec %ecx
	jmp 1b

2:	mov $_bp,%edx	
	push %edx
	call main
	addl $4,%esp

halt:
	hlt
/*
 * Should never been here, if we are here, something goes wrong
 */
	int $0x18
1:
	jmp 1b
jump:
	mov jump_addr,%ebx
	jmp *%ebx

.data
drive_ref:
	.long 0x00000000

