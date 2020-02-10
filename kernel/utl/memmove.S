/*
 * Kerner
 *
 * Author       : Sebastien LAURENT
 * Date         : 29/09/2011
 *
 * memmove
 */

.intel_syntax noprefix
.code64

.global memmove

.text

memmove:
	push rbp
	mov rbp,rsp

	mov rax,rdi
	mov rcx,rdx	

	cmp rdi,rsi
	jge 2f
	cld	
	jmp 3f
2:	
	std
	add rdi,rcx
	add rsi,rcx
	dec rsi
	dec rdi
3:
	rep movsb

	pop rbp
	ret

