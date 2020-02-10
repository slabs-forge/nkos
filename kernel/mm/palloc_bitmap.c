/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 07/10/2011
 *
 * Physical bitmap allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"

#include "kernel/mm/pg.h"
#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_impl.h"
#include "kernel/mm/palloc_bitmap.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"
#include "kernel/utl/bitscan.h"

#define MAX_ORDER 11

/*
 * Structures definition
 */

struct zone_t {
	struct zone_t* prev;
	struct zone_t* next;

	void* base;

	struct {
		uint64_t* start;
		uint64_t count;
	} bmap[MAX_ORDER];

	uint64_t busy;
};

struct header_t {
	void* mbase;

	struct zone_t* head;
	struct zone_t* tail;	
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

static void _setup_zone(struct header_t*,void*,size_t, struct palloc_t*); 
static void _page_reserve(struct header_t*,void*);
static void _zone_set_busy(struct zone_t*,void*,int64_t order);
static void _zone_set_free(struct zone_t*,void*,int64_t order);

/*
 * Static data
 */
static struct header_t _data = {
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
	.name = "Bitmap Physical Allocator",
	.data = &_data,
	.vptr = &_vtbl,
};

/*
 * Constructor
 */
struct palloc_t* palloc_bitmap_create() {
	return &_singleton;
}

/* 
 * Return first address in zone slot i
 */
static inline uint64_t _zone_bgn(struct zone_t* zone,uint64_t i) {
	uint64_t a = (1ULL << (PAGE_LOG_SIZE + i)) - 1;
	uint64_t b = ((uint64_t) zone->base + a) & ~a;
	return b;
}

/*
 * Return last address in zone slot i
 */
static inline uint64_t _zone_end(struct zone_t* zone,uint64_t i) {
	uint64_t a = (1ULL << (PAGE_LOG_SIZE + i)) - 1;
	uint64_t b = ((uint64_t) zone->base + a) & ~a;
	uint64_t e = b + (zone->bmap[i].count << (PAGE_LOG_SIZE + i));
	return e;
}

/*
 * Return page frame number in slot i
 */
static inline uint64_t _zone_pfn(struct zone_t* zone, void* addr,uint64_t i) {
	uint64_t a = (1ULL << (PAGE_LOG_SIZE + i)) - 1;
	uint64_t b = ((uint64_t) zone->base + a) & ~a;
	return ((uint64_t)addr - b) >> (PAGE_LOG_SIZE + i);
}

/*
 * Initialization
 */
static void _init(struct palloc_t* new,struct palloc_t* old) {
	struct header_t* data = new->data;

	auto void __zone_init(void* base,size_t size);
	auto void __zone_reserve(void* base,size_t size);

	void __zone_init(void* base,size_t size) {
		_setup_zone(data,base,size,old);
	}

	auto void __zone_reserve(void* base,size_t size) {
		void* p;

		p = (void*)(((uint64_t)base + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

		while (p + PAGE_SIZE < base+size) {
			_page_reserve(data,p);
			p += PAGE_SIZE;
		}
	}

	/*
 	 * Initialize base of mapping
	 */
	data->mbase = (void*)(0xFFFF000000000000ULL | 511ULL<<39);

	palloc_foreach_zone(old,__zone_init);
	palloc_foreach_page(old,__zone_reserve);
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
	struct header_t* data = allocator->data;
}

/*
 * Debug info
 */
static void _debug(struct palloc_t* allocator) {
	struct header_t* header = allocator->data;

	uint64_t total = 0;
	uint64_t busy = 0;
	for (struct zone_t* zone = header->head; zone != 0; zone = zone->next) {
		kdebug("Zone %016lx-%016lx",zone->base,zone->base+(zone->bmap[0].count << PAGE_LOG_SIZE));
		kdebug(" %ld / %ld\n", zone->busy, zone->bmap[0].count);
		
		busy += zone->busy;
		total += zone->bmap[0].count;

		for (uint64_t i = 0 ; i < MAX_ORDER; i++) {
			uint64_t b = _zone_bgn(zone,i);
			uint64_t e = _zone_end(zone,i);

			if (zone->bmap[i].count == 0) break;
			kdebug(" [%2d]",i);
			kdebug(" %016lx", b);
			kdebug(" %016lx", e);
			kdebug(" %016lx", zone->bmap[i].start);
			kdebug(" %ld pages", zone->bmap[i].count);
			kdebug("\n");
		}
	}
	kdebug("Core size      : %10ld kbytes\n", total * 4);
	kdebug("Allocated size : %10ld kbytes\n", busy * 4);
}

/*
 * Zone Setup
 */
static void _setup_zone(struct header_t* data,void* base,size_t size,struct palloc_t* allocator) {
	struct zone_t* zone;
	uint64_t offs;
	uint64_t sz;

	uint64_t bmsz[MAX_ORDER];

	offs = sz = (sizeof(struct zone_t) + 7ULL) & ~7ULL;

	memset(bmsz,0,sizeof(uint64_t) * MAX_ORDER);

	for (int16_t i = 0; i < MAX_ORDER; i++) {
		uint64_t c;
		uint64_t a = (1 << (PAGE_LOG_SIZE + i)) - 1;
		uint64_t b = ((uint64_t)base + a) & ~a;
		uint64_t e = ((uint64_t)base + size) & ~a;
		if (b >= e) break;

		/*
		 * Compute page count
		 */
		bmsz[i] = (e - b) >> (PAGE_LOG_SIZE + i);
		
		/*
		 * Compute quad count
	         */
		sz += ((bmsz[i] + 63) & ~63) >> 3;
	}

	/*
	 * Round up sz to page size
	 */
	sz = ((sz + PAGE_SIZE - 1) & ~(PAGE_SIZE-1)) >> PAGE_LOG_SIZE;

	zone = data->mbase;

	for (uint64_t i = 0; i < sz ; i ++) {
		void* addr = palloc_get(allocator,0);
		pg_map(data->mbase,addr,allocator);
		memset(data->mbase,0, PAGE_SIZE);
		data->mbase += PAGE_SIZE;
	}

	/*
	 * Initialize Zone
	 */
	for (uint64_t i = 0 ; i < MAX_ORDER; i++) {
		zone->bmap[i].start = (uint64_t*)((void*)zone +offs);
		zone->bmap[i].count = bmsz[i];
		offs += ((bmsz[i] + 63) & ~63) >> 3;
	}

	zone -> base =  (void*)(((uint64_t)base + PAGE_SIZE - 1) & ~ (PAGE_SIZE - 1));
	/*
	 * Add the zone
	 */
	zone->prev = data->tail;
	zone->next = 0;
	data->tail = zone;
	if (zone->prev != 0) {
		zone->prev->next = zone;	
	} else {
		data->head = zone;
	}
}

/*
 * Page reservation (4kb size)
 */
static void _page_reserve(struct header_t* header,void* addr) {
	struct zone_t* zone;
	
	for (zone = header->head; zone != 0 ; zone = zone->next) {
		if ((uint64_t)addr < _zone_bgn(zone,0)) continue;
		if ((uint64_t)addr + PAGE_SIZE >= _zone_end(zone,0)) continue;
		
		/*
		 * Ok, that s the right zone
		 */
		for (uint64_t i ; i < MAX_ORDER; i++) {
			uint64_t a = (1ULL << (PAGE_LOG_SIZE + i)) -1;
			void* p = (void*)((uint64_t)addr & ~a);

			if ( (uint64_t)p < _zone_bgn(zone,i)) break;
			if ( (uint64_t)p >= _zone_end(zone,i)) break;
			_zone_set_busy(zone,p,i);
		}
		zone->busy++;
		break;
	}
}

/*
 * Fix l'etat d'une page a occupe
 */
static void _zone_set_busy(struct zone_t* zone,void* p,int64_t order) {
	uint64_t pfn = _zone_pfn(zone,p,order);
	zone->bmap[order].start[pfn >> 6] |= (1ULL << (pfn % 64));
}

/*
 * Fix l'etat d'une page a libre
 */
static void _zone_set_free(struct zone_t* zone,void* p,int64_t order) {
	uint64_t pfn = _zone_pfn(zone,p,order);
	zone->bmap[order].start[pfn >> 6] &= ~(1ULL << (pfn % 64));
}

/*
 * Renvoie l'etat d'une page
 */
static uint64_t _zone_get_busy(struct zone_t* zone,void* p,int64_t order) {
	uint64_t pfn = _zone_pfn(zone,p,order);
	return  (zone->bmap[order].start[pfn>>6] >> (pfn%64)) & 0x1ULL;
}

/*
 * Phys. Page Allocation
 */
static void* _alloc(struct palloc_t* allocator,uint64_t order) {
	struct header_t* header = (struct header_t*) allocator->data;

	for (struct zone_t* zone = header->head; zone != 0 ; zone = zone->next) {
		uint64_t c = zone->bmap[order].count;

		c = ((c + 63ULL) & ~63ULL) >> 6;
		for (uint64_t i = 0 ; i < c ; i++) {
			void* addr;
			uint64_t pfn;

			if (~zone->bmap[order].start[i] == 0) continue;

			pfn = bscan_forward(~zone->bmap[order].start[i]);
			pfn += (i << 6);	

			if (pfn >= zone->bmap[order].count) break;

			addr = (void*)_zone_bgn(zone,order) + (pfn << (PAGE_LOG_SIZE + order));
			_zone_set_busy(zone,addr,order);

			//kdebug("Found in zone %016lx pfn=%ld addr=%016lx\n",zone->base,pfn,addr);

			for (uint64_t j = 0 ; j < order ; j++) {
				uint64_t e = (uint64_t)addr + (1ULL << (PAGE_LOG_SIZE + order));
				uint64_t s = 1ULL << (PAGE_LOG_SIZE + j);
				for (uint64_t b = (uint64_t)addr; b < e; b += s) {
					_zone_set_busy(zone,(void*)b,j);
				}
			}

			for (uint64_t j = order + 1; j < MAX_ORDER ; j++) {
				uint64_t a = 1ULL << (PAGE_LOG_SIZE + j);
				uint64_t b = (uint64_t)addr & ~(a - 1);

				if (zone->bmap[j].count == 0) break;
				if (b < _zone_bgn(zone,j)) break;
				if (b >= _zone_end(zone,j)) break;
				if (_zone_get_busy(zone,(void*)b,j)) break;

				_zone_set_busy(zone,(void*)b,j);
			}

			zone->busy += 1ULL << order;
			return addr;
		}
	}

	return 0;
}

/*
 * Free pages
 */
static void _free(struct palloc_t* allocator,void* addr,size_t order) {
	struct header_t* header = allocator->data;
	uint64_t a = 1ULL << (PAGE_LOG_SIZE + order);

	ASSERT( ((uint64_t)addr & (a - 1)) == 0);

	for (struct zone_t* zone = header->head; zone != 0 ; zone = zone->next) {
		if (zone->bmap[order].count == 0) continue;
		if ((uint64_t)addr < _zone_bgn(zone,order)) continue;
		if ((uint64_t)addr >= _zone_end(zone,order)) continue;

		_zone_set_free(zone,addr,order);
		
		for (uint64_t j = 0 ; j < order ; j++) {
			uint64_t e = (uint64_t)addr + (1ULL << (PAGE_LOG_SIZE + order));
			uint64_t s = 1ULL << (PAGE_LOG_SIZE + j);
			for (uint64_t b = (uint64_t)addr; b < e; b += s) {
				//kdebug("Set free %016lx (%ld)\n",b,j);
				_zone_set_free(zone,(void*)b,j);
			}
		}

		for (uint64_t j = order + 1 ; j < MAX_ORDER; j++) {
			uint64_t a = 1ULL << (PAGE_LOG_SIZE + j);
			uint64_t b0 = (uint64_t)addr & ~ (a-1);
			uint64_t b1 = b0 + (1ULL << (PAGE_LOG_SIZE + j - 1));

			if (zone->bmap[j].count == 0) break;
			if (b0 < _zone_bgn(zone,j)) break;
			if (b0 >= _zone_end(zone,j)) break;

			//kdebug("b0 = %016lx v=%ld\n",b0,_zone_get_busy(zone,(void*)b0,j-1));
			//kdebug("b1 = %016lx v=%ld\n",b0,_zone_get_busy(zone,(void*)b1,j-1));
			if (_zone_get_busy(zone,(void*)b0,j-1) || _zone_get_busy(zone,(void*)b1,j-1)) break;
			
			//kdebug("Updating bitmap %ld : %016lx %016lx\n",j,b0,b1);	
			_zone_set_free(zone,(void*)b0,j);
		}

		zone->busy -= (1ULL << order);
		return;
	}
	/*
	 * Did not found corresponding zone
	 */
	ASSERT(0);
}
