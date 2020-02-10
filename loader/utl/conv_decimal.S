/*
 * NK Loader - Utility functions
 *
 * Author	: Sebastien LAURENT
 * Date		: 02/09/2011
 *
 * Conversion int32 vers chaine en decimal
 */

.code32

.global conv_decimal

.text
	
conv_decimal:
	push %ebp
	mov %esp,%ebp
	push %edi
	push %ebx

	mov 16(%ebp),%ecx
	or %ecx,%ecx
	jz err

	mov 12(%ebp),%edi
	add %ecx,%edi

	xor %al,%al
	dec %edi
	mov %al,(%edi)
	dec %ecx
	or %ecx,%ecx
	jz err

	xor %ebx,%ebx
	mov $10,%bl
	mov 8(%ebp),%eax
5:
	xor %edx,%edx
	div %ebx
	add $'0',%dl
	dec %edi
	mov %dl,(%edi)
	dec %ecx
	or %eax,%eax
	jz ok
	or %ecx,%ecx
	jz err
	jmp 5b

err:	
	xor %edi,%edi
ok:
	mov %edi,%eax

	mov 20(%ebp),%edi
	or %edi,%edi
	jz 1f
	mov %ecx,(%edi)
1:	pop %ebx
	pop %edi
	pop %ebp
	ret

