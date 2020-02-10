/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * memset
 */

.code32

.global memset

.text

memset:
	push %ebp
	mov %esp,%ebp
	push %edi
	cld
	mov 8(%ebp),%edx
	mov %edx,%edi
	mov 12(%ebp),%eax
	mov 16(%ebp),%ecx
	rep stosb
	mov %edx,%eax
	pop %edi
	pop %ebp
	ret

