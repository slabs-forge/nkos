/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * Amorce
 */

.intel_syntax noprefix
.global _start
.global switch_pm
.code32

.section ".head"
_start:
	call start
1:
.align 4
/*
 * Boot Data Block
 */

	/*
	 * _start + 8: relocation segment
	 */
	.word 0x8000 

	/*
	 * _start + 10: relocation size
	 */
	.long _size

	/*
	 * _start + 14
	 * Prefered video screen X size
	 */
	.word 1280

	/*
	 * _start + 16
	 * Prefered video screen X size
	 */
	.word 1024

	/*
	 * _start + 18
	 * Initial page directory address
	 */
	.long 0x70000

	/*
	 * Start + 22
	 * Kernel Physical Load Address
	 */
	.quad 0x0

	/*
	 * Start + 30 
	 * Offset of kernel
	 */
	.quad 0x0

	/*
	 * Start +38
	 * Size of kernel
	 */
	.quad 0x0
	/*
	 * Memory region (64 entries)
	 */
	.fill 20*65
.text

start:	
	/*
	 * Relocate the code above 1Mbytes limit
	 */
	pop esi
	sub esi,(1b - _start)

	mov edx,esi

	/*
	 * Relocate code before 1 Mbytes 
	 */
	xor eax,eax
	mov ax,[esi+8]
	mov dx,ax
	shl eax,4
	mov edi,eax
	mov ecx,[esi+10]
	shr ecx,1
	cld
	push esi
	rep movsw
	pop esi

	/*
	 * Time machine... back in 1950....
	 */
	mov eax,0x00000018
	push eax
	mov eax,esi
	add eax,OFFSET 1f
	push eax
	retf
1:
	/*
	 * Now in 16 bit segment
	 */
.code16gcc
	mov eax,cr0
	and eax,0xFFFFFFFE
	mov cr0,eax
	
	/* 
	 * Let's prepare data segment
	 */
	mov ds,dx
	mov es,dx
	mov fs,dx
	mov gs,dx
	
	/*
	 * Make a nice stack
	 */
	mov ss,dx
	xor eax,eax
	mov ax,0xFFFC
	mov esp,eax
	
	mov eax,OFFSET 2f
	push edx
	push eax
	retf	
2:
	/*
	 * TODO: clear BSS
	 */
	cli
	xor eax,eax
	xor ebx,ebx
	xor ecx,ecx
	xor edx,edx
	xor esi,esi
	xor edi,edi
	
	/*
	 * Jump to C code
	 */
	call main
3:	nop
	hlt
	jmp 3b
switch_pm:
	/*
	 * Getting bootparm offset in ebx
	 */
	xor ebx,ebx
	mov ebp,esp
	mov ebx,[ebp+4]

	/*
	 * switching pm
	 */
	mov eax,cr0
	or eax,0x1
	mov cr0,eax

	/*
	 * ebp will store base segment adress in 32 bit format
	 * add it to ebx to get a abosulute adress for bootparm
	 */
	mov ebp,cs
	shl ebp,4
	add ebx,ebp

	/*
	 * Jump in 32 bit segment
	 */
	mov eax,0x00000008
	push eax
	xor eax,eax
	mov eax,ebp
	add eax, OFFSET 1f
	push eax
	retf
.code32
1:	
	/* 
	 * Load new data32 segment
	 */
	mov ax,0x0010
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	/*
	 * Adjust stack
	 */
	mov ss,ax
	add esp,ebp
2:	
	/*
	 * Clearing pages directories
	 */
	mov edi,[ebp+18]
	xor eax,eax
	mov ecx,0x2000
	push edi
	rep stosd
	pop edi

	/*
	 * Setting up PML4 for first 512Go and last for kernel
	 */
	mov eax,edi
	add eax,0x1000
	or eax,0x3
	mov [edi],eax
	add eax,0x3000
	mov [edi+0xFF8],eax

	/* 
	 * Setting up PDPTE for first Go
	 */
	add edi,0x1000 
	mov eax,edi
	add eax,0x1000
	or eax, 0x3
	mov [edi],eax

	/*
	 * Setting up PDE for first 2Mo
	 */
	add edi,0x1000
	mov eax,edi
	add eax,0x1000
	or eax,0x3
	mov [edi],eax

	/*
	 * Setting up PTE
	 */
	add edi,0x1000
	xor eax,eax
	xor edx,edx
	mov ecx,512
	or eax,0x3
1:	mov [edi],eax
	mov [edi+4],edx
	add eax,0x1000
	add edi,8
	loopnz 1b

	/*
	 * Setting up kernel PDTE
	 */
	mov eax,edi
	add eax,0x1000
	or eax,0x3
	mov [edi+0xFF8],eax
	add eax,0x2000
	mov [edi+0xFF0],eax

	/*
	 * Setting kernel pde
	 */
	add edi,0x1000
	mov eax,edi
	add eax,0x1000
	or eax,0x3
	mov [edi],eax
	
	/*
	 * Setting high pte
	 */
	add edi,0x1000
	mov eax,0x100000
	add eax,[ebp+22]
	or eax,0x3

	mov ecx,[ebp+30]

1:	mov [edi],eax
	add eax,0x1000
	sub ecx,0x1000
	add edi,8
	or ecx,ecx
	jnz 1b
		
	/*
	 * Now initial stack 
	 */
	mov edi,[ebp+18]
	add edi,0x7000

	/*
	 * PDE
	 */
	mov eax,edi
	add eax,0x1000
	or eax,0x03
	mov [edi+0xFF8],eax
	
	/* 
	 * And finally PT
	 */
	add edi,0x1000
	mov eax,0x100003
	mov [edi+0xFF8],eax
	
	mov edi,[ebp+18]

	/*
	 * Set PAE & PGE
	 */
	mov eax,0x000000A0
	mov cr4,eax
	
	/*
	 * Set page directory location
	 */
	mov cr3,edi

	/*
	 * Set EFER.LME
	 */
	mov ecx,0xC0000080
	rdmsr
	or eax,0x00000100
	wrmsr

	/*
	 * Set PG
	 */
	mov eax,cr0
	or eax,0x80000000
	mov cr0,eax

	mov eax,0x18
	push eax
	mov eax,OFFSET 2f
	add eax,ebp
	push eax
	retf
2:	
	/*
	 * We re now in 64 bit mode
	 */
.code64
	mov ax,0x20
	mov ss,ax
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax

	/*
	 * Reusing first page of loaded kernel
	 */
	mov rsp,0xFFFFFFFFC0000000

	/*	
	 * Great JUMP !!!!!!!!!!!!!
	 * ebp points to kernel real relocation
	 */
	xor rdi,rdi
	mov edi,ebx
	xor rsi,rsi
	mov esi,0x100000
	add esi,[rbp+22]

	mov rax,0xFFFFFFFFC0000000
	jmp rax
	hlt	
.data
.bss
.align 4
mm:	
