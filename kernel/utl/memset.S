/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * memset
 */

.intel_syntax noprefix
.code64

.global memset

.text

memset:
	push rbp
	mov rbp,rsp
	cld
	mov rax,rsi
	mov rsi,rdi
	mov rcx,rdx
	rep stosb
	mov rax,rsi
	pop rbp
	ret

