/*
 * Kernel - amorce
 *
 * Author       : Sebastien LAURENT
 * Date         : 20/09/2011
 *
 * far_strcpy
 */

.intel_syntax noprefix

.global far_strncpy

.code16gcc
.text

far_strncpy:
	push ebp
	mov ebp,esp
	push edi
	push esi
	push ds
	
	mov edi,[ebp+8]
	mov eax,[ebp+12]
	mov ecx,[ebp+16]
	mov edx,edi
	cld

	xor esi,esi
	mov si,ax
	shr eax,16
	mov ds,ax

1:	lodsb
	stosb
	or al,al
	jz 2f
	loopne 1b
2:
	mov eax,edx
	pop ds
	pop esi
	pop edi
	pop ebp
	ret

