/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * memmove
 */

.code32

.global memmove

.text

memmove:
	push %ebp
	mov %esp,%ebp
	push %edi
	push %esi
	
	mov 8(%ebp),%edi
	mov 12(%ebp),%esi
	mov 16(%ebp),%ecx

	cmp %esi,%edi
	jge 2f
	cld	
	jmp 3f
2:	
	std
	add %ecx,%edi
	add %ecx,%esi
	dec %esi
	dec %edi
3:
	rep movsb

	pop %esi
	pop %edi
	pop %ebp
	ret

