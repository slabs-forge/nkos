/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * Support CPUID
 */

.intel_syntax noprefix
.global cpuid
.global cpuid_support

.text
.code16gcc 
/*
 * Test cpuid support
 * eax = 1 if supported
 */
cpuid_support:
	push ebp
	mov ebp,esp
	
	pushfd
	pop eax
	mov ecx,eax
	xor eax, 1 << 21
	push eax
	popfd
	pushfd
	pop eax
	xor eax,ecx
	push ecx
	popfd
	shr eax,21
	
	pop ebp
	ret

/*
 * cpuid
 *
 * [ebp+8]	uint32_t in
 * [ebp+12]	out struct cpuid_t
 */
cpuid:
	push ebp
	mov ebp,esp
	push esi
	push ebx

	mov eax,[ebp+8]
	cpuid
	mov esi,[ebp+12]

	mov [esi],eax
	mov [esi+4],ebx
	mov [esi+8],ecx
	mov [esi+12],edx

	pop ebx
	pop esi
	pop ebp
	ret
