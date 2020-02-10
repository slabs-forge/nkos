/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 13/10/2011
 *
 * Physical Buddy Allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"

#include "kernel/mm/pg.h"
#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_impl.h"
#include "kernel/mm/palloc_buddy.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"

#define MAX_ORDER 11

/*
 * Structures definition
 */


// Memory Region
struct region_t {
	const char* name;
	void* base;
	uint64_t size;
};

// Allocator main data
struct header_t {
	uint32_t nregs;
	struct region_t* regs;
};

/*
 * Prototypes
 */
static void _init(struct palloc_t*,struct palloc_t*);
static void* _alloc(struct palloc_t*,size_t size);
static void _free(struct palloc_t*,void*,size_t);
static void _debug(struct palloc_t*);
static void _foreach_zone(struct palloc_t*,void (*)(void*,size_t));
static void _foreach_page(struct palloc_t*,void (*)(void*,size_t));

static void _reserve(struct header_t*,void* base,size_t size);
static void _setup_zone(struct header_t*,uint64_t,uint64_t);
static void _setup_zone_in_region(struct header_t*,struct region_t*,uint64_t,uint64_t);

/*
 * Static data
 */

struct region_t _regs[] = {
	{ .base = (void*)0x0000000000000000ULL , .size = 0x0000000001000000ULL , .name = "DMA"},
	{ .base = (void*)0x0000000001000000ULL , .size = 0x00000000FF000000ULL , .name = "NORMAL"},
	{ .base = (void*)0x0000000100000000ULL , .size = 0x00003FFF00000000ULL , .name = "HIGH"},
};

static struct header_t _data = {
	.nregs = sizeof(_regs) / sizeof (struct region_t),
	.regs = _regs,
};

static struct palloc_vtbl_t _vtbl = {
	.init = _init,
	.alloc = _alloc,
	.free = _free,
	.debug = _debug,
	.foreach_zone = _foreach_zone,
	.foreach_page = _foreach_page,
};

static struct palloc_t _singleton = {
	.name = "Buddy Physical Allocator",
	.data = &_data,
	.vptr = &_vtbl,
};

/*
 * Constructor
 */
struct palloc_t* palloc_buddy_create() {
	return &_singleton;
}

/*
 * Initialization
 * 
 * -Mark the page map
 * -Scan adress
 */
static void _init(struct palloc_t* new,struct palloc_t* old) {
	struct header_t* header = new->data;

	auto void __zone_init(void* base,size_t size);
	auto void __zone_reserve(void* base,size_t size);

	void __zone_init(void* base,size_t size) {
		_setup_zone(header,(uint64_t)base,(uint64_t)base + size);
	}

	auto void __zone_reserve(void* base,size_t size) {
		_reserve(header,base,size);
	}

	palloc_foreach_page(old,__zone_reserve);
	palloc_foreach_zone(old,__zone_init);
}

/*
 * Iteration sur pages alloues
 */
static void _foreach_page(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
}

/*
 * Iteration sur zones
 */
static void _foreach_zone(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
}

/*
 * Debug info
 */
static void _debug(struct palloc_t* allocator) {
	struct header_t* header = allocator->data;
}

/*
 * Phys. Page Allocation
 */
static void* _alloc(struct palloc_t* allocator,uint64_t order) {
	struct header_t* header = (struct header_t*) allocator->data;
	ASSERT(0);
	return 0;
}

/*
 * Free pages
 */
static void _free(struct palloc_t* allocator,void* addr,size_t order) {
	struct header_t* header = allocator->data;
	ASSERT(0);
}

/*
 * Mark reserved zone
 */
static void _reserve(struct header_t* header,void* base,size_t size) {
	for (void* p = base; p < base + size; p+= PAGE_SIZE) {
		struct page_t* page = pg_pma2page(p);
		page->type = PAGE_TYPE_SYSTEM;
		page->order = 0;
	}

}

/*
 * Add an e820 zone 
 */
static void _setup_zone(struct header_t* header,uint64_t bzone,uint64_t ezone) {
	for (uint32_t i = 0 ; i < header->nregs ; i ++) {
		uint64_t b = (uint64_t)header->regs[i].base;
		uint64_t e = (uint64_t)header->regs[i].base + header->regs[i].size;

		if (bzone >= e) continue;

		if (bzone < b) {
			if (ezone > b && ezone <= e) {
				_setup_zone_in_region(header,&header->regs[i],b,ezone);
			} else if (ezone > e) {
				_setup_zone_in_region(header,&header->regs[i],b,e);
			}
		} else if (bzone >= b && bzone < e) {
			if (ezone <= e) {
				_setup_zone_in_region(header,&header->regs[i],bzone,ezone);
			} else {
				_setup_zone_in_region(header,&header->regs[i],bzone,e);
			}
		}
	}
}

/*
 * Ajoute une zone dans une region
 */
static void _setup_zone_in_region(struct header_t* header,struct region_t* region,uint64_t bzone,uint64_t ezone) {
	uint8_t type = 0x01;
	uint64_t b;
	uint64_t e;
	uint64_t a;

	kdebug("Adding %016lx %016lx in zone %s\n",bzone,ezone,region->name);

	for (a = bzone; a < ezone; a += PAGE_SIZE) {
		struct page_t* page = pg_pma2page((void*)a);

		if (page->type == 0x00 && type != 0x00) {
			type = page->type;
			b =a;			
		}

		if (page->type != 0x00 && type == 0x00) {
			type = page->type;
			e = a;
			kdebug("-add %016lx %016lx\n",b,e);
		}
	}
	if (type == 0x00) {
			e = a;
			kdebug("-add %016lx %016lx\n",b,e);
	}
}

/*
 * TODO:
 *
 * -Deal with DMA Zone, all memory allocated here for machine with lots of memory
 * -split of zone
 */

