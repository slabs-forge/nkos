/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * strncmp
 */

.intel_syntax noprefix

.code32

.global strncmp

.text

strncmp:
	push ebp
	mov ebp,esp
	push edi
	push esi
	push ebx
	
	mov esi,[ebp+8]
	mov edi,[ebp+12]
	mov ebx,[ebp+16]
	xor edx,edx
	xor eax,eax
	xor ecx,ecx

1:	cmp ecx,ebx
	je 2f	
	mov al,[esi+ecx]
	mov dl,[edi+ecx]
	sub eax,edx
	jne 2f
	inc ecx
	and edx,edx
	jnz 1b

2:	
	pop ebx
	pop esi
	pop edi
	pop ebp
	ret


