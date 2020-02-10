/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 06/10/2011
 *
 * Physical bootstrap allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"

#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_impl.h"
#include "kernel/mm/palloc_bootstrap.h"

#include "kernel/utl/kdebug.h"

/*
 * Structures definition
 */
struct palloc_bootstrap_data_t {
	struct e820_t* mem;
};

/*
 * Prototypes
 */
static void* _alloc(struct palloc_t*,size_t);
static void _debug(struct palloc_t*);
static void _foreach_zone(struct palloc_t*,void (*)(void*,size_t));
static void _foreach_page(struct palloc_t*,void (*)(void*,size_t));

/*
 * Static data
 */
static struct palloc_bootstrap_data_t _data __attribute__((section(".data"))) = {
};

static struct palloc_vtbl_t _vtbl = {
	.debug = _debug,
	.alloc = _alloc,
	.foreach_zone = _foreach_zone,
	.foreach_page = _foreach_page,
};

static struct palloc_t _singleton = {
	.name = "Bootstrap Physical Allocator",
	.data = &_data,
	.vptr = &_vtbl,
};

/*
 * Constructor
 */
struct palloc_t* palloc_bootstrap_create(struct bootparm_t* bp) {
	_data.mem = bp->mem;
	return &_singleton;
}

/*
 * Iteration sur les pages alloues
 */
static void _foreach_page(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
}

/*
 * Iteration sur les zones
 */
static void _foreach_zone(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
        uint64_t head;
        uint64_t tail;

        struct palloc_bootstrap_data_t* data = allocator->data;

        for (struct e820_t* p = data->mem; p->size != 0 ; p++) {
                if (p->type != 1) continue;

                head = (uint64_t)p->base;
                tail = p->size + head;

                head = (head + PAGE_SIZE -1) & ~ (PAGE_SIZE-1);
                tail = tail & ~ (PAGE_SIZE-1);

		callback((void*)head,tail - head);
        }
}

/*
 * Debug info
 */
static void _debug(struct palloc_t* allocator) {
	auto void __debug_zone(void* base,size_t size);

	void __debug_zone(void* base,size_t size) {
		kdebug("Zone %016lx-%016lx (%ld pages)\n", base, base + size, size / PAGE_SIZE);
	}

	_foreach_zone(allocator,__debug_zone);
}

/*
 * Allocation
 */
static void* _alloc(struct palloc_t* allocator,size_t size) {
	ASSERT(0);
}
