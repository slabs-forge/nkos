/*
 * Kernel - Amorce
 *
 * Author       : Sebastien LAURENT
 * Date         : 21/09/2011
 *
 * memset
 */

.intel_syntax noprefix
.code16gcc

.global memset

.text

memset:
	push ebp
	mov ebp,esp
	push edi
	cld
	mov edx,[ebp+8]
	mov edi,edx
	mov eax,[ebp+12]
	mov ecx,[ebp+16]
	rep stosb
	mov eax,edx
	pop edi
	pop ebp
	ret

