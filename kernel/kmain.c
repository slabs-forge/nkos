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
#include "kernel/bootparm.h"
#include "kernel/e820.h"
#include "kernel/interrupts.h"
#include "kernel/assert.h"
#include "kernel/cpuinfo.h"

#include "kernel/mm/pg.h"
#include "kernel/mm/zone.h"
#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_bootstrap.h"
#include "kernel/mm/palloc_contiguous.h"
#include "kernel/mm/palloc_bitmap.h"
#include "kernel/mm/palloc_buddy.h"
#include "kernel/mm/slab.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"

/*
 * Return head of kernel reserved physical address
 */
static inline void* _kernel_phead() {
	return (void*)0x100000;
}

/*
 * Return tail of kernel reserved physical address
 */
static inline void* _kernel_ptail() {
	ASSERT(0);
	return 0x0;
}

/*
 * Setup initial paging
 */
void init_paging(struct palloc_t* allocator,void* base) {
	size_t sz = __end - __text;

	struct pd_t* pml4 = (struct pd_t*) palloc_get(allocator,1);
	struct pd_t* pdpt = (struct pd_t*) palloc_get(allocator,1);
	struct pd_t* pd = (struct pd_t*) palloc_get(allocator,1);
	struct pd_t* pt = 0;

	uint64_t i2 = 0;
	uint64_t i1 = 0;

	memset(pml4,0,sizeof(struct pd_t));
	memset(pdpt,0,sizeof(struct pd_t));
	memset(pd,0,sizeof(struct pd_t));

	/*
	 * Initialize kernel mapping
	 */
	pml4->pages[511] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_RW;
	pdpt->pages[511] = (uint64_t) pd | PAGE_PRESENT | PAGE_RW; 

	for (void* p = base; p < base + sz; p += PAGE_SIZE) {
		if (pt == 0 || i1 == 512) {
			/*
			 * create new Level 1 pd
			 * and register it in the level2 pd
			 */
			pt =(struct pd_t*) palloc_get(allocator,1);	
			memset(pt,0,sizeof(struct pd_t));

			pd->pages[i2++] = (uint64_t) pt | PAGE_PRESENT | PAGE_RW;
			i1 = 0;
		}
		pt->pages[i1++] = (uint64_t) p | PAGE_PRESENT | PAGE_RW;
	}

	/*
	 * Self reference pml4
	 */
	pml4->pages[PML4_SELF] = (uint64_t) pml4 | PAGE_PRESENT | PAGE_RW;

	/*
	 * Prepare a 16ko kernel stack
	 */
	pd = (struct pd_t*) palloc_get(allocator,1);
	memset(pd,0,sizeof(struct pd_t));
	pdpt->pages[510] = (uint64_t) pd | PAGE_PRESENT | PAGE_RW;
	pt = (struct pd_t*) palloc_get(allocator,1);
	memset(pt,0,sizeof(struct pd_t));
	pd->pages[511] = (uint64_t) pt | PAGE_PRESENT | PAGE_RW;

	/*
	 * first stack page was setted up in the launcher, simply reuse it
	 */
	pt->pages[511] = (uint64_t) 0x100000ULL | PAGE_PRESENT | PAGE_RW;
	pt->pages[510] = (uint64_t) palloc_get(allocator,1) | PAGE_PRESENT | PAGE_RW;
	pt->pages[509] = (uint64_t) palloc_get(allocator,1) | PAGE_PRESENT | PAGE_RW;
	pt->pages[508] = (uint64_t) palloc_get(allocator,1) | PAGE_PRESENT | PAGE_RW;

	pg_load_pml4(pml4);
	kdebug("Paging updated\n");
}

/*
 * build direct mapping
 */
void build_direct_mapping(struct palloc_t* allocator,struct e820_t *m) {
	uint64_t base = 0;
	uint64_t size = 0;

	for (int64_t i = 0 ; i < E820_ENTRIES; i++) {
		if (m->size == 0) break;

		if (base + size == m->base) {
			size+=m->size;
		} else {
			if (size > 0) pg_map_region_optimized((void*)0xffff800000000000ULL+base,(void*)base,size,allocator);
			
			base=m->base;
			size=m->size;
		} 

		m++;
	}
	if (size > 0) pg_map_region_optimized((void*)0xffff800000000000ULL+base,(void*)base,size,allocator);
}

/*
 * Build page map
 */
void build_page_map(struct palloc_t* allocator,struct e820_t *m) {
	uint64_t npages = 0;
	uint64_t size;
	uint64_t order;

	void *v;
	void *p;

	for (uint64_t i = 0; i < E820_ENTRIES; i++,m++) {
		if (m->size == 0) break;
		if (m->type != 1) continue;

		if (m->base + m->size > npages) npages = m->base + m->size;
	}
	npages >>= PAGE_LOG_SIZE;

	size = npages * sizeof(struct page_t);
	size = (size + PAGE_SIZE-1) & ~ (PAGE_SIZE-1);

	v=(void*)VMAP_ADDR;

	for (order = MAX_ORDER - 1; size !=0;) {
		uint64_t s;

		while (order > 0 && (1ULL << (PAGE_LOG_SIZE + order)) > size) order--;

		s = 1ULL << (PAGE_LOG_SIZE + order);
		p = palloc_get(allocator,order);

		//kdebug("Vmap %016lx %016lx %016lx\n",v ,p , s);
		pg_map_region_optimized(v,p,s,allocator);
		memset(v,0,s);

		v += s;		
		size -= s;	
	}
}

/*
 * we are mapped at FFFFFFFFC0000000
 *
 * - kernel bootstrap console for debugging
 *
 * At start:
 * -paged first 2 Mo
 * -paged .text and .data from kernel at 0x100000+start offset
 *
 */
void kmain(void* bp, void* kb) {
	struct palloc_t* allocator;

	void* pbss;

	/*
	 * It's an early init of serial line so we can send debug output
	 */
	serial_init();

	kdebug("Kernel is starting...\n");
	kdebug("boot parameter   : %016lx\n",bp);
	kdebug("kernel phys addr : %016lx\n",kb);
	kdebug("_text            : %016lx\n",__text);
	kdebug("_data            : %016lx\n",__data);
	kdebug("_bss             : %016lx\n",__bss);
	kdebug("_end             : %016lx\n",__end);

	/*
	 * Copy the boot parameter & store the Physical Load Addr of Kernel
	 */
	memmove(kbootparm(),bp,sizeof(struct bootparm_t));
	kbootparm()->kernel_pla = (uint64_t)kb;

	/*
	 * Cleanup of e820 memory region
	 */
	e820_init(kbootparm());

	/*
	 * Zones initialization
	 */
	zone_init();

	/*
	 * Create bootstrap allocator
	 */
	allocator = palloc_bootstrap_create(kbootparm());
	allocator = palloc_init(palloc_contiguous_create(),allocator);

	/* 
	 * Reserve space for bss
	 */
	pbss = palloc_get(allocator,(__end - __bss) >> PAGE_LOG_SIZE);

	/*
	 * Reseting IDT
	 */
	lidt((void*)0x0,0x0);

	/*
	 * Set up paging
	 * 
	 * - Kernel mapping (with .bss)
	 * - Self PML4 recursive mapping
	 * - 16kb stack
	 */
	init_paging(allocator,kb);

	/*
	 * At the stage we can access .bss, but we must zero it
	 */
	memset((void*)__bss,0,__end - __bss);

	/*
	 * Collect CPU infos
	 */
	cpuinfo_init();

	ASSERT(0);
	/*
	 * Do the direct mapping
	 */
	build_direct_mapping(allocator,kbootparm()->mem);

	/*
	 * Build Page Map
 	 */
	build_page_map(allocator,kbootparm()->mem);

	/*
	 * At the stage we need a better allocator
	 * Let's switch to bitmap allocator
	 */
	allocator = palloc_init(palloc_buddy_create(),allocator);
	palloc_debug(allocator);
	ASSERT(0);

	/*
	 * We have
	 * - .bss ok
	 * - stack ok
	 * - paging ok
	 * - direct mapping ok
	 * - page directory ok
	 */

	/*
	 * Boot the slab allocator
	 */
	slab_init(allocator);

	struct slab_cache_t* cache = slab_register("pipo_t",535,sizeof(uint64_t),0,0);	

	for (uint64_t i = 0; i < 128 ; i++) {
		void* a = slab_allocate(cache);
		kdebug("a=%016lx\n",a);
	}
	slab_debug();
	palloc_debug(allocator);
	ASSERT(0);
	
	/*
 	 * TO DO:
	 * - slab large
	 * - slab free
	 * - fianl phys alloc
	 * - kmalloc & co
	 * IDT & co
	 */
}


