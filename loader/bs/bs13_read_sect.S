/*
 * NK Loader - Bios Services
 *
 * Author       : Sebastien LAURENT
 * Date         : 06/09/2011
 *
 * Read sectors
 */

.intel_syntax noprefix

.text

.global bs13_read_sect

.code32

bs13_read_sect:
	push ebp
	mov ebp,esp
	push esi
	push edi
	push ebx

	call switch_rm
.code16gcc
3:
	mov esi,OFFSET dap

	/*
	 * Load LBA Adress into DAP
	 */
	mov eax,[ebp+12]
	mov [esi+8],eax
	mov edx,[ebp+16]
	mov [esi+12],edx

	/*
	 * Compute segment:offset
	 */
	mov edi,[ebp+24]
	mov ecx,edi
	and ecx,0xFFFF0000
	shr ecx,4
        and edi,0x0000FFFF
	mov WORD PTR [esi+6],cx
	mov WORD PTR [esi+4],di


	/*
	 * See if anymore sector to read
	 */	
	mov ecx,[ebp+20]
	or ecx,ecx
	jz ok

	/*
	 * 64 sector at a time
	 */
	cmp ecx,64
	jle 1f
	mov ecx,64
1:
	mov WORD PTR [esi+2],cx
	sub [ebp+20],ecx

	/*
	 * Compute next LBA
	 */
	add eax,ecx
	adc edx,0
	mov [ebp+12],eax
	mov [ebp+16],edx
	
	/*
	 * Compute next address
	 */
	shl ecx,9
	add [ebp+24],ecx
	
	mov ah,0x42
	mov edx,[ebp+8]
	int 0x13
	jnc 3b
ko:
	shr eax,8
	and eax,0xFF
	jmp 2f
ok:
	xor eax,eax
2:
	push eax
	call switch_pm
.code32
	pop eax
	pop ebx
	pop edi
	pop esi
	pop ebp
	ret
.data
dap:
        .byte 0x10              // Size of dap
        .byte 0x00              // Reserved
        .word 0x0000            // Num of sector
        .word 0x0000            // Offset
        .word 0x1000            // Segment
        .long 0x00000000        // Position (Low)
        .long 0x00000000        // Position (High)

