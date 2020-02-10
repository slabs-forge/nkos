/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 06/10/2011
 *
 * Physical contiguous allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"

#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_impl.h"
#include "kernel/mm/palloc_contiguous.h"

#include "kernel/utl/kdebug.h"

#define ZONES_MAX 32

/*
 * Structures definition
 */
struct palloc_contiguous_data_t {
	struct {
		void* base;
		size_t size;
	} zones[ZONES_MAX];
	uint16_t zcnt;
	uint16_t zidx;
	void* pbgn;
	void* pend;
	
};

/*
 * Prototypes
 */
static void _init(struct palloc_t*,struct palloc_t*);
static void* _alloc(struct palloc_t*,size_t size);
static void _debug(struct palloc_t*);
static void _foreach_zone(struct palloc_t*,void (*)(void*,size_t));
static void _foreach_page(struct palloc_t*,void (*)(void*,size_t));

/*
 * Static data
 */
static struct palloc_contiguous_data_t _data __attribute__((section(".data"))) = {
	.pbgn = (void*) -1LL,
	.pend = (void*) -1LL,
};

static struct palloc_vtbl_t _vtbl = {
	.init = _init,
	.alloc = _alloc,
	.debug = _debug,
	.foreach_zone = _foreach_zone,
	.foreach_page = _foreach_page,
};

static struct palloc_t _singleton = {
	.name = "Contiguous Physical Allocator",
	.data = &_data,
	.vptr = &_vtbl,
};

/*
 * Constructor
 */
struct palloc_t* palloc_contiguous_create() {
	return &_singleton;
}

/*
 * Initialization
 */
static void _init(struct palloc_t* new,struct palloc_t* old) {
	struct palloc_contiguous_data_t* data = new->data;
	uint64_t pla = kbootparm()->kernel_pla;

	auto void __register(void* base,size_t size);

	void __register(void* base,size_t size) {
		data->zones[data->zcnt].base = base;
		data->zones[data->zcnt].size = size;
		data->zcnt++;

		ASSERT(data->zcnt <= ZONES_MAX);
	}

	data->zcnt = 0;
	palloc_foreach_zone(old,__register);

	for (uint16_t i = 0 ; i < data->zcnt; i++) {
		if ((uint64_t)data->zones[i].base > pla && data->zones[i].base < data->pbgn) {
			data->zidx = i;
			data->pbgn = data->zones[i].base;
		}
	}
	data->pend = data->pbgn;
}

/*
 * Phys. Page Allocation
 */
static void* _alloc(struct palloc_t* allocator,uint64_t order) {
	uint64_t size;
	void* addr = 0;

	struct palloc_contiguous_data_t* data = allocator->data;

	size = 1ULL << (order + PAGE_LOG_SIZE);

	if (data->pend + size < data->zones[data->zidx].base + data->zones[data->zidx].size) {
		addr = data->pend;
		data->pend += size;
	}
	return addr;
}

/*
 * Iteration sur pages alloues
 */
static void _foreach_page(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
	struct palloc_contiguous_data_t* data = allocator->data;

	if (data->pbgn != data->pend) {
		callback(data->pbgn,data->pend - data->pbgn);
	}
}

/*
 * Iteration sur zones
 */
static void _foreach_zone(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
	struct palloc_contiguous_data_t* data = allocator->data;

	for (uint16_t i = 0 ; i < data->zcnt ; i++) {
		callback(data->zones[i].base,data->zones[i].size);
	}
}

/*
 * Debug info
 */
static void _debug(struct palloc_t* allocator) {
	auto void __zone_print(void* base,size_t size);
	auto void __page_print(void* base,size_t size);

	void __zone_print(void* base, size_t size) {
		kdebug("Zone %016lx-%016lx (%ld pages)\n", base, base + size, size / PAGE_SIZE);	
	}

	void __page_print(void* base,size_t size) {
		kdebug("Allocated pages: %016lx-%016lx (%ld pages)\n", base, base + size, size / PAGE_SIZE);
	}

	_foreach_zone(allocator,__zone_print);
	_foreach_page(allocator,__page_print);
}

