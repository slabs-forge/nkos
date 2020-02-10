/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * Appel INT
 */

.intel_syntax noprefix
.global intcall

.text
.code16gcc 
/*
 * [ebp+8]	int number
 * [ebp+12]	in registers
 * [ebp+16]	out registers
 */
intcall:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ebx
	
	/*
	 * store int number (self modifing code)
	 */
	mov eax,[ebp+8]
	mov 1f,al
 
	jmp 2f
2:
	/*
	 * load registers
	 */
	mov esi,[ebp+12]

	mov eax,[esi]
	mov ebx,[esi+4]
	mov ecx,[esi+8]
	mov edx,[esi+12]
	mov edi,[esi+20]
	mov esi,[esi+16]
	.byte 0xCD
1:	.byte 0x00

	mov esi,[ebp+16]
	or esi,esi
	jz 3f
	mov [esi],eax
	pushfd
	pop eax
	mov [esi+24],eax
	mov [esi+4],ebx
	mov [esi+8],ecx
	mov [esi+12],edx
	mov [esi+20],edi
3:
	pop ebx
	pop edi
	pop esi
	pop ebp
	ret

