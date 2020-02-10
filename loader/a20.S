/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * A20 Line manipulation
 */

.code16gcc

.global a20_test
.global a20_disable
.global a20_enable

.text

/*
 * test the a20 line
 */
a20_test:
	push %ebp
	mov %esp,%ebp
	push %ds
	push %es
	push %di
	push %si
		
	mov $0x7DFE,%si
	mov $0x7E0E,%di

	xor %ax,%ax
	mov %ax,%ds
	not %ax
	mov %ax,%es

	mov %es:(%di),%ax
	mov %ds:(%si),%cx
	cmp %ax,%cx
	jne 1f
	/*
	 * C'est egal, on confirme...
	 */
	notw %es:(%di)
	notw %ds:(%si)
	mov %es:(%di),%ax
	mov %ds:(%si),%cx
	notw %es:(%di)
	notw %ds:(%si)
	cmp %ax,%cx
	jne 1f

	xor %eax,%eax
	not %eax
	jmp 2f
1:
	xor %eax,%eax
2:

	pop %si
	pop %di
	pop %es
	pop %ds
	pop %ebp
	ret

/*
 * Disable the A20 line
 */
a20_disable:
	push %ebp
	mov %esp,%ebp
	cli

	call a20_wait
	mov $0xd1,%al
	out %al,$0x64
	call a20_wait
	mov $0xdd,%al
	out %al,$0x60

	sti
	pop %ebp
	ret

/*
 * Enable the A20 line
 */
a20_enable:
	push %ebp
	mov %esp,%ebp
	cli

	call a20_wait
	mov $0xd1,%al
	out %al,$0x64
	call a20_wait
	mov $0xdf,%al
	out %al,$0x60

	sti
	pop %ebp
	ret

/*
 * Wait for the Keyboard Controller
 */
a20_wait:
	in $0x64,%al
	test $2,%al
	jnz a20_wait
	ret	

