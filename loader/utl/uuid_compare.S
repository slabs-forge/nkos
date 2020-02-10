/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * uuid_compare
 */

.intel_syntax noprefix

.code32

.global uuid_compare

.text

uuid_compare:
	push ebp
	mov ebp,esp
	push edi
	push esi
	
	xor eax,eax
	mov ecx,4
	mov esi,[ebp+8]
	mov edi,[ebp+12]
	cld
1:	cmpsd
	jne 2f
	loop 1b
	jmp 3f
2:	dec eax
3:	pop esi
	pop edi
	pop ebp
	ret


