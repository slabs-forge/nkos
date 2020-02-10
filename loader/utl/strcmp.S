/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * strcmp
 */

.intel_syntax noprefix

.code32

.global strcmp

.text

strcmp:
	push ebp
	mov ebp,esp
	push edi
	push esi
	
	mov esi,[ebp+8]
	mov edi,[ebp+12]
	xor edx,edx
	xor eax,eax
	xor ecx,ecx

1:	mov al,[esi+ecx]
	mov dl,[edi+ecx]
	sub eax,edx
	jne 2f
	inc ecx
	and edx,edx
	jnz 1b

2:	
	pop esi
	pop edi
	pop ebp
	ret


