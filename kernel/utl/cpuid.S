/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 12/10/2011
 *
 * Support CPUID
 */

.intel_syntax noprefix
.global cpuid

.text
.code64 
/*
 * cpuid
 *
 * [rdi]	in/out struct cpuid_t
 */
cpuid:
	push rbp
	mov rbp,rsp
	push rbx

	mov eax,[rdi]
	mov ebx,[rdi+4]
	mov ecx,[rdi+8]
	mov edx,[rdi+12]

	cpuid

	mov [rdi],eax
	mov [rdi+4],ebx
	mov [rdi+8],ecx
	mov [rdi+12],edx

	pop rbx
	pop rbp
	ret
