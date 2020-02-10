/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * strlen function
 */

.intel_syntax noprefix
.code64

.global strlen

.text
strlen:
	push rbp
	mov rbp,rsp

	cld
	mov rcx,rdi
	xor al,al
1:	scasb
	jne 1b
	sub rdi,rcx
	mov rax,rdi

	pop rbp
	ret
