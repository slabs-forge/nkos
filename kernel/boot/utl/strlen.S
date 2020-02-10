/*
 * Kernel - launcher
 *
 * Author       : Sebastien LAURENT
 * Date         : 22/09/2011
 *
 * strlen function
 */

.intel_syntax noprefix
.code16gcc

.global strlen

.text

strlen:
	push ebp
	mov ebp,esp
	push edi

	cld
	mov edi,[ebp+8]
	xor al,al
1:	scasb	
	jne 1b
	sub edi,[ebp+8]
	mov eax,edi
	dec eax
	pop edi
	pop ebp
	ret

