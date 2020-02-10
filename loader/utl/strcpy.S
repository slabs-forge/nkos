/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * strcpy
 */

.intel_syntax noprefix

.code32

.global strcpy

.text

strcpy:
	push ebp
	mov ebp,esp
	push edi
	push esi
	
	mov edi,[ebp+8]
	mov esi,[ebp+12]
	mov eax,edi
	cld
	
1:	lodsb
	stosb
	or al,al
	jnz 1b

	pop esi
	pop edi
	pop ebp
	ret


