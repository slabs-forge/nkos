/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * hexadecimal conversions
 */

.code32

.global conv_hexa

.text

conv_hexa:
	push %ebp
	mov %esp,%ebp
	push %edi

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

	mov 8(%ebp),%edx
	
3:	mov %dl,%al
	and $0x0f,%al
	cmp $0x0a,%al
	jge 1f
	add $'0',%al
	jmp 2f
1:
	add $('a'-10),%al
	jmp 2f
2:	
	dec %edi
	movb %al,(%edi)
	or %ecx,%ecx
	jz err
	dec %ecx
	shr $4,%edx
	or %edx,%edx
	jnz 3b
		
	jmp ok
err:
	xor %edi,%edi
ok:	mov %edi,%eax

	pop %edi
	pop %ebp
	ret

