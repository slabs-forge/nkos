/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/10/2011
 *
 * Slab Allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/cpuinfo.h"

#include "kernel/mm/pg.h"
#include "kernel/mm/slab.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"
#include "kernel/utl/bitscan.h"

/*
 * Structure definitions
 */
struct sa_t {
	struct slab_cache_t* head;
	struct slab_cache_t* tail;
	struct palloc_t* allocator;

	struct slab_cache_t* cache_slab;
	struct slab_cache_t* cache_buf;

	uint64_t cache_line;
};

struct buf_t {
	struct slab_t* slab;
	struct buf_t* next;
	void* data;
};

struct slab_t {
	void* page;
	struct slab_cache_t* cache;
	struct slab_t* next;
	struct slab_t* prev;
	struct buf_t* free;
	uint16_t bsy;
	uint16_t cnt;
};

struct slab_cache_t {
	struct slab_cache_t* prev;
	struct slab_cache_t* next;

	struct slab_t* slab_free;
	struct slab_t* slab_busy;
	struct slab_t* slab_full;

	const char* name;

	uint32_t osize;
	uint16_t nobjs;
	uint16_t order;
	uint16_t align;
	uint16_t offset;
	uint16_t colors;
	uint16_t nxtcol;

	uint64_t bsy;
	uint64_t cnt;
};

/*
 * Prototypes
 */
static void _slab_cache_init(struct slab_cache_t*,const char*,uint64_t,uint16_t);
static void _slab_grow_small(struct slab_cache_t*);
static void _slab_grow_large(struct slab_cache_t*);

/*
 * Inlines
 */
static inline uint64_t _slab_header_size(uint64_t n,uint64_t a) {
	uint64_t r = sizeof(struct slab_t);
	r += n * sizeof(struct buf_t);
	r = (r + a - 1) & ~ (a - 1);
	return r;
}

static inline void _slab_grow(struct slab_cache_t* cache) {
	if (cache->osize * 8 < PAGE_SIZE) {
		_slab_grow_small(cache);
	} else {
		_slab_grow_large(cache);
	}
}

/*
 * Static data
 */
static struct sa_t _sa = {
};

static struct slab_cache_t _ca;

/*
 * Slab Allocator Initialisation
 */
void slab_init(struct palloc_t* allocator) {
	kdebug("Slab Allocator Initialization\n");

	_sa.allocator = allocator;	
	_sa.head = &_ca;
	_sa.tail = &_ca;
	_sa.cache_line = cpuinfo_get_cache_line(CPU_CACHE_LEVEL_L1,CPU_CACHE_TYPE_DATA);

	_slab_cache_init(&_ca,"slab_cache_t", sizeof(struct slab_cache_t), sizeof(uint64_t));

	/*
	 *  Registering own slab
	 */
	_sa.cache_slab = slab_register("slab_t",sizeof(struct slab_t),sizeof(uint64_t),0,0);
	_sa.cache_buf = slab_register("buf_t",sizeof(struct buf_t),sizeof(uint64_t),0,0);
}

/*
 * Cache registration
 */
struct slab_cache_t* slab_register(const char* name,uint64_t size,uint16_t align,void (*ctor)(void*),void (*dtor)(void*)) {
	struct slab_cache_t* cache = slab_allocate(&_ca);

	_slab_cache_init(cache, name, size, align);

	/*
	 * Add in the cache list
	 */
	cache->prev = _sa.tail;
	cache->next = 0;
	if (cache->prev) cache->prev->next = cache;
	_sa.tail = cache;

	return cache;
}

/*
 * Debug info on Slab Allocator
 */
void slab_debug() {
	kdebug("Slab allocator debugging info:\n");

	kdebug("      Busy |      Total |       size | Al | Ord. |   ops | Name\n");
	kdebug("-----------+------------+------------+----+------+-------+-------\n");
	for (struct slab_cache_t* cache = _sa.head ; cache != 0 ; cache = cache->next) {
		kdebug("%10d",cache->bsy);
		kdebug(" | %10d",cache->cnt);
		kdebug(" | %10d",cache->osize);
		kdebug(" | %2d",cache->align);
		kdebug(" | %4d",cache->order);
		kdebug(" | %5d",cache->nobjs);
		kdebug(" | %s",cache->name);
		kdebug("\n");
	}
}

/*
 * Allocation
 */
void* slab_allocate(struct slab_cache_t* cache) {
	struct slab_t* slab;
	struct buf_t* buf;

	if (cache->slab_busy == 0 && cache->slab_free == 0) {
		_slab_grow(cache);
	}

	if (cache->slab_busy != 0) {
		slab = cache->slab_busy;
	} else {
		slab = cache->slab_free;
	}

	/*
	 * Detach next free buff
	 */
	buf = slab->free;
	slab->free = buf->next;
	buf->next = 0;

	slab->bsy++;
	cache->bsy++;
	
	if (slab == cache->slab_free) {
		/*
		 * Move the slab to busy list
		 */
		cache->slab_free = slab->next;
		if (slab->next) slab->next->prev = 0;

		slab->next = cache->slab_busy;
		if (slab->next) slab->next->prev = slab;
		cache->slab_busy = slab;
	} else if (slab->bsy == slab->cnt) {
		/*
		 * Move the slab to full list
		 */
		cache->slab_busy = slab->next;
		if (slab->next) slab->next->prev = 0;

		slab->next = cache->slab_full;
		if (slab->next) slab->next->prev = slab;
		cache->slab_full = slab;
	}
	return buf->data;
}

/*
 * Small cache growing up
 */
static void _slab_grow_small(struct slab_cache_t* cache) {
	struct slab_t* slab;
	struct buf_t* buf;
	struct buf_t* prv;
	void* data;
	void* page;

	page = palloc_get(_sa.allocator,cache->order);
	ASSERT(page != 0);

	page = pg_pma2vma(page);

	/*
	 * Compute slab&data position
	 */
	slab = (struct slab_t*)(page + cache->offset * cache->nxtcol);
	data = (void*)slab + _slab_header_size(cache->nobjs,cache->align);

	/*
	 * Cycle color
	 */
	cache->nxtcol++;
	if (cache->nxtcol >= cache->colors) cache->nxtcol = 0;

	/*
	 * Slab initialization
	 */
	slab->page = page;
	slab->cache = cache;
	slab->prev = 0;
	slab->bsy = 0;
	slab->cnt = cache->nobjs;

	/*
	 * buf initialization
	 */
	buf = (struct buf_t*)(slab + 1);
	prv = 0;

	for (uint16_t i = 0; i < cache->nobjs ; i++) {
		buf[i].slab = slab;
		buf[i].data = data + i * cache->osize;
		buf[i].next = prv;

		prv = &buf[i];
	}
	slab->free = prv;
 
	cache->cnt += slab->cnt;

	/*
	 * Insert the slab in the free list
	 */
	slab->next = cache->slab_free;
	if (slab->next) slab->next->prev = slab;
	cache->slab_free = slab;
}

/*
 * Large cache growing up
 */
static void _slab_grow_large(struct slab_cache_t* cache) {
	struct slab_t* slab;
	void* page;
	void* data;

	page = palloc_get(_sa.allocator,cache->order);
	ASSERT(page!=0);

	page = pg_pma2vma(page);

	/*
	 * Compute ata position
	 */
	data = page + cache->offset * cache->nxtcol;

	/*
	 * Cycle color
	 */
	cache->nxtcol++;
	if (cache->nxtcol >= cache->colors) cache->nxtcol = 0;

	slab = (struct slab_t*) slab_allocate(_sa.cache_slab);
	ASSERT(slab!=0);

	slab->page = page;
	slab->cache = cache;
	slab->prev = 0;
	slab->free = 0;
	slab->cnt = cache->nobjs;
	slab->bsy = 0;

	for (uint16_t i = 0; i < slab->cnt; i++) {
		struct buf_t* buf = slab_allocate(_sa.cache_buf);

		buf->slab = slab;
		buf->next = slab->free;
		buf->data = data;

		data += cache->osize;
		slab->free = buf;
	}

	cache->cnt += slab->cnt;

	/*
	 * Insert the slab in the free list
	 */
	slab->next = cache->slab_free;
	if (slab->next) slab->next->prev = slab;
	cache->slab_free = slab;
}

/*
 * Cache initialization
 */
void _slab_cache_init(struct slab_cache_t* cache,const char* name,uint64_t size,uint16_t align) {
	uint64_t rsize;

	memset(cache,0,sizeof(struct slab_cache_t));

	/*
	 * Slab managed object limited to MAX_ORDER page order size
	 */
	ASSERT(size < (PAGE_SIZE << MAX_ORDER));

	/*
	 * Alignment limited to PAGE_SIZE
	 */
	if (align >= PAGE_SIZE) align = PAGE_SIZE;
	if (align <= sizeof(uint64_t)) align = sizeof(uint64_t);
	align = (align + sizeof(uint64_t) - 1) & ~(sizeof(uint64_t) - 1);

	cache->name = name;

	cache->align = align;
	cache->osize = (uint32_t)((size + align - 1) & ~ (align - 1));

	if (cache->osize * 8 < PAGE_SIZE) {
		/*
		 * For small page, ON SLAB storage
		 */
		cache->order = 0;

		cache->nobjs = (uint16_t)((PAGE_SIZE - sizeof(struct slab_cache_t)) / (sizeof(struct buf_t) + cache->osize));
	
		/*
		 * Must adjust nobjs accroding to alignment
		 */
		rsize = _slab_header_size(cache->nobjs,cache->align);
		rsize += cache->nobjs * cache->osize;
		if (rsize > PAGE_SIZE) {
			cache->nobjs--;
			rsize = _slab_header_size(cache->nobjs,cache->align);
			rsize += cache->nobjs * cache->osize;
		}
		rsize = PAGE_SIZE - rsize;
	} else {
		uint8_t order;
		uint64_t tsize;
		uint64_t nobjs;

		for (order = 0; order < MAX_ORDER; order++) {
			uint64_t tsize = PAGE_SIZE << order;

			if (cache->osize > tsize) continue;

			nobjs = tsize / cache->osize;
			rsize = tsize - nobjs * cache->osize;

			if ( (rsize << 3) < tsize) {
				break;
			}
		}
		if (order == MAX_ORDER) order--;

		cache->order = order;
		cache->nobjs = nobjs;
	}
	cache->offset = _sa.cache_line;
	cache->offset += align -1;
	cache->offset &= ~ (align -1); 
	cache->colors = (uint16_t) rsize / cache->offset + 1;
	cache->nxtcol = 0;
}

