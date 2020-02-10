/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * strlen function
 */

.code32

.global strlen

.text

strlen:
	push %ebp
	mov %esp,%ebp
	push %edi

	cld
	mov 8(%ebp),%edi
	xor %al,%al
1:	scasb	
	jne 1b
	sub 8(%ebp),%edi
	mov %edi,%eax
	dec %eax
	pop %edi
	pop %ebp
	ret

