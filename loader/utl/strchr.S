/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * strchr
 */

.code32

.global strchr

.text

strchr:
	push %ebp
	mov %esp,%ebp
	push %edi
	xor %eax,%eax
	mov 8(%ebp),%edi
	mov 12(%ebp),%edx
3:	mov (%edi),%dh
	or %dh,%dh
	jz 2f
	cmp %dh,%dl
	je 1f
	inc %edi
	jmp 3b
1:
	mov %edi,%eax
2:	pop %edi
	pop %ebp
	ret
