/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 26/09/2011
 *
 * Kernel main
 */

#include "kernel/types.h"
#include "kernel/serial.h"
#include "kernel/kernel.h"

#include "kernel/mm/pg.h"
#include "kernel/mm/palloc.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"

static uint64_t _base[] = {
        PT_BASE,
        PD_BASE,
        PDPT_BASE,
        PML4_BASE
};

static uint64_t _offs[] = {
        0x7ffffffULL << 12,
        0x3ffffULL << 12,
        0x1ffULL << 12,
        0x0ULL
};

/*
 * Prototypes
 */
static void _pg_map(void*,void*,int64_t,struct palloc_t*);


/*
 * Mapping with 4kb page
 */
void pg_map(void* v,void* p,struct palloc_t* allocator) {
	_pg_map(v,p,0,allocator);
}

/*
 * Mapping with 2Mb page
 */
void pg_map_large(void* v,void* p,struct palloc_t* allocator) {
	_pg_map(v,p,1,allocator);
}

/*
 * Mapping with Level L Page
 */
static void _pg_map(void* v,void* p,int64_t l,struct palloc_t* allocator) {
	struct pd_t* pdt = (struct pd_t*) PML4_ADDR;

	uint64_t k;
	uint64_t s = 4096 << (l*9);
	int64_t i;

	//kdebug("pg_map [%016lx] %016lx-%016lx > %016lx-%016lx\n",s, p , p+s-1, v , v+s-1);

	for (i=3; i > l ; i--) {
		k = ((uint64_t)v >> (12 + 9*i)) & 0x1ffUL;
		if (pdt->pages[k] == 0) {
			pdt->pages[k] = (uint64_t) palloc_get(allocator,0) | PAGE_PRESENT | PAGE_RW;
			kdebug("Creating L-%ld PD at %016lx\n",i,pdt->pages[k]);
			pdt = pg_vma2pgdir(v,i-1);	
			memset(pdt,0,PAGE_SIZE);
			pg_flush_tlb(pdt);
		} else {
			pdt = pg_vma2pgdir(v,i-1);
		}
	}
	k = ((uint64_t)v >> (12 + 9*i)) & 0x1ffUL;
	//kdebug("Create mapping l=%d %016lx %016lx\n",l,p,&pdt->pages[k]);
	pdt->pages[k] = (uint64_t)p | PAGE_PRESENT | PAGE_RW | ( l == 0 ? 0 : PAGE_PS);
	pg_flush_tlb((void*)p);
}

/*
 * Mapping d'une region en utilisant au besoin des pages de 2Mb
 */
void pg_map_region_optimized(void* v,void* p,uint64_t s,struct palloc_t* allocator) {
	uint64_t ps;

	kdebug("Map %016lx-%016lx",p,p+s);
	kdebug(" to %016lx-%016lx\n",v,v+s);

	/*
	 * Le mapping se fait en VMA-PMA croissante
	 */
	for (; s > 0; p += ps, v+= ps, s -= ps) {
		if ((((uint64_t)p & 0x1fffffULL) == 0) && (((uint64_t)v & 0x1fffffULL) == 0) && (s >= 0x200000ULL)) {
			_pg_map(v,p,1,allocator);
			ps = 0x200000ULL;
		} else {
			_pg_map(v,p,0,allocator);
			ps = 0x1000ULL;
		}
	}
}

/*
 * Mapping d'une region
 */
void pg_map_region(void* v,void* p,uint64_t s,struct palloc_t* allocator) {
	uint64_t ps = 0x1000ULL;

	kdebug("Map %016lx-%016lx",p,p+s);
	kdebug(" to %016lx-%016lx\n",v,v+s);

	/*
	 * Le mapping se fait en VMA-PMA croissante
	 */
	for (; s > 0; p += ps, v+= ps, s -= ps) {
		_pg_map(v,p,0,allocator);
	}
}

/*
 * Convert VMA address to page directory level l 
 */
uint64_t* pg_vma2pgent(const void* v,int64_t l) {
        uint64_t* pde;

        pde = (uint64_t*)(_base[l] | (uint64_t)v >> (9 + l*9) & (_offs[l] | 0xff8ULL) );

        return pde;
}

/*
 * Convert VMA address to page directory level l 
 */
struct pd_t* pg_vma2pgdir(const void* v,int64_t l) {
        struct pd_t* pdt;

        pdt = (struct pd_t*)(_base[l] | (uint64_t)v >> (9 + l*9) & _offs[l] );

        return pdt;
}

/*
 * Conversion from VMA to PMA
 */
void* pg_vma2pma(const void* vma) {
        uint64_t* pde;

        pde = pg_vma2pgent(vma,3);
        if ((*pde & PAGE_PRESENT) == 0) return 0;
        if (*pde & PAGE_PS) return (void*)(*pde & ~PAGE_MASK);

        pde = pg_vma2pgent(vma,2);
        if ((*pde & PAGE_PRESENT) == 0) return 0;
        if (*pde & PAGE_PS) return (void*)(*pde & ~PAGE_MASK);

        pde = pg_vma2pgent(vma,1);
        if ((*pde & PAGE_PRESENT) == 0) return 0;
        if (*pde & PAGE_PS) return (void*)(*pde & ~PAGE_MASK | ((uint64_t) vma & 0x1fffffULL));

        pde = pg_vma2pgent(vma,0);
        if ((*pde & PAGE_PRESENT) == 0) return 0;

        return (void*)(*pde & ~PAGE_MASK | (uint64_t)vma & 0xFFF);
}

