/*
 * NK Loader - Bios Services
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Read drive parameters
 */

.text

.global bs13_get_drive_parms

.code32

bs13_get_drive_parms:
	push %ebp
	mov %esp,%ebp
	push %esi

	call switch_rm
.code16gcc
	mov 8(%ebp),%edx
	mov 12(%ebp),%esi
	movw $0x001A,(%esi)

	mov %esi,%ecx
	and $0xFFFF0000,%ecx
	shr $4,%ecx
	and $0x0000FFFF,%esi

	xor %eax,%eax
	mov $0x48,%ah

	push %ds
	mov %cx,%ds
	int $0x13
	pop %ds
	jc 1f
	xor %eax,%eax
	jmp 2f
1:
	shr $8,%eax
	neg %eax

2:	push %eax
	call switch_pm
.code32
	pop %eax
	pop %esi
	pop %ebp
	ret
