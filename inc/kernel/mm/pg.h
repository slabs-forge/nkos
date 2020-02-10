/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 04/10/2011
 *
 * Memory mapper
 */

#ifndef __KERNEL_MM_MM_H__
#define __KERNEL_MM_MM_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

#define PAGE_PRESENT    0x01
#define PAGE_RW         0x02
#define PAGE_PS         0x80
#define PAGE_MASK       0x8000000000000fffULL

#define VMAP_ADDR       ( 0xffff000000000000ULL | 506ULL << 39)

#define PAGE_TYPE_NULL		0
#define PAGE_TYPE_SYSTEM	1
#define PAGE_TYPE_BUDDY		2
#define PAGE_TYPE_SLAB		3

#define PML4_SELF       510ULL
#define PML4_ADDR       ( 0xffff000000000000ULL \
                        | PML4_SELF << 39 \
                        | PML4_SELF << 30 \
                        | PML4_SELF << 21 \
                        | PML4_SELF << 12 \
                        )

#define PML4_BASE       PML4_ADDR
#define PDPT_BASE       ( 0xffff000000000000ULL \
                        | PML4_SELF << 39 \
                        | PML4_SELF << 30 \
                        | PML4_SELF << 21 \
                        )

#define PD_BASE         ( 0xffff000000000000ULL \
                        | PML4_SELF << 39 \
                        | PML4_SELF << 30 \
                        )

#define PT_BASE         ( 0xffff000000000000ULL \
                        | PML4_SELF << 39 \
                        )

struct page_t {
	uint8_t type;
	uint8_t flags;
	uint8_t order;
	uint8_t reserved10;
	uint32_t reserved11;
        void* reserved2;
        void* reserved3;
        void* reserved4;
	void* reserved5;
	void* reserved6;
	void* reserved7;
	void* reserved8;
} __attribute__((packed));

struct pd_t {
        uint64_t pages[512];
};

/*
 * Convert Phys. Addr to Virt. Addr
 */
static inline void* pg_pma2vma(void* addr) {
        return (void*)addr + 0xffff800000000000ULL;
}

/*
 * Invalidate all TLBs
 */
static inline void pg_flush_all_tlb() {
        asm(    "mov %%cr3,%%rax; mov %%rax,%%cr3;"
                :
                :
                : "rax"
        );
}

/*
 * Load PML4
 */
static inline void pg_load_pml4(void* addr) {
        asm( "mov %%rax,%%cr3;"
        : /* No Output */
        : "a" (addr)
        );
}

/*
 * Invalidate TLBs
 */
static inline void pg_flush_tlb(void* addr) {
        asm( "invlpg %0"
                :
                : "m" (*(char*)addr)
        );
}

/*
 * Get page_t descriptor
 */
static inline struct page_t* pg_pma2page(void* p) {
	uint64_t addr = VMAP_ADDR + ((uint64_t)p >> PAGE_LOG_SIZE) * sizeof(struct page_t);
	return (struct page_t*)addr;
}

void pg_map(void*,void*,struct palloc_t*);
void pg_map_large(void*,void*,struct palloc_t*);

void pg_map_region(void*,void*,uint64_t,struct palloc_t*);
void pg_map_region_optimized(void*,void*,uint64_t,struct palloc_t*);

uint64_t* pg_vma2pgent(const void*,int64_t);
struct pd_t* pg_vma2pgdir(const void*,int64_t);
void* pg_vma2pma(const void*);

#endif

