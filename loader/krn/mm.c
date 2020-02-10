/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Memory Manager
 */

#include "loader/types.h"
#include "loader/module.h"

#include "loader/krn/mm.h"

#include "loader/utl/stdio.h"

/*
 * Allocator (simplified implementation of dlmalloc)
 */

#define MM_START	0x00010000
#define MM_END		0x000A0000

#define PBUSY_BIT	0x00000001
#define CBUSY_BIT	0x00000002
#define PBUSY_MSK	0xFFFFFFFE
#define CBUSY_MSK	0xFFFFFFFD

#define SIZE_MSK	0xFFFFFFFC
#define FLAG_MSK	0x00000003

#define MAX_SMALL	0x00000100
#define SB_COUNT	32
#define LB_COUNT	96

#if defined(MM_DEBUG)
#define MM_DEBUG(FORMAT,...) printf(FORMAT, ## __VA_ARGS__)
#else
#define MM_DEBUG(FORMAT,...) 
#endif

/*
 * chunks are stored in bins according to their size
 *
 * <256 bytes 	-> small chunks
 * >=256 bytes	-> large chunks
 *
 * There's 4 large bins per power of two.
 * For example, chunks from 1024 to 2048 are stored in 4 bins:
 *
 * 1024 - 1280
 * 1280 - 1536
 * 1536 - 1792
 * 1792 - 2048
 *
 * So there's (32-8) * 4 = 96 large bins
 *
 * Small bin are stored by increment of 8, so there's 32 bins
 */

struct chunk_t;
struct arena_t;

void mm_debug();
void mm_debug_chunk(struct chunk_t*);

static inline size_t mm_align4(size_t);
static inline size_t mm_align8(size_t);

static inline void mm_set_csize(struct chunk_t* p,size_t size);
static inline void mm_set_psize(struct chunk_t* p,size_t size);
static inline void mm_set_cbusy(struct chunk_t* p);
static inline void mm_set_pbusy(struct chunk_t* p);
static inline void mm_clr_cbusy(struct chunk_t* p);
static inline void mm_clr_pbusy(struct chunk_t* p);
static inline size_t mm_get_csize(struct chunk_t* p);
static inline size_t mm_get_psize(struct chunk_t* p);

static inline int mm_is_cbusy(struct chunk_t* p);
static inline int mm_is_pbusy(struct chunk_t* p);

static inline struct chunk_t* mm_first();
static inline struct chunk_t* mm_last();
static inline struct chunk_t* mm_next(struct chunk_t*);
static inline struct chunk_t* mm_prev(struct chunk_t*);

static inline void* mm_chunk2data(struct chunk_t*);
static inline struct chunk_t* mm_data2chunk(void*);

static inline void mm_link(struct chunk_t*);
static void mm_link_small(struct chunk_t*);
static void mm_link_large(struct chunk_t*);

static inline void mm_unlink(struct chunk_t*);
static void mm_unlink_small(struct chunk_t*);
static void mm_unlink_large(struct chunk_t*);

static size_t mm_slot_large(size_t);
static size_t mm_key_large(size_t);

static struct chunk_t* mm_find_small(size_t);
static struct chunk_t* mm_find_large(size_t);

DECLARE_BUILTIN_MODULE("mm",mm_init,40000);

/*
 * Chunk data
 */
struct chunk_t {
	/*
	 * previous chunk size
	 */
	size_t psz;

	/*
	 * current chunk size and busy bits
	 * bit 0 : previous chunk in use
	 * bit 1 : current chunk in use
	 */
	size_t csz;

	/*
	 * small chunk data
	 */
	struct chunk_t* prev;
	struct chunk_t* next;

	/* 
	 * large chunk data
	 * p 	: parent
	 * c[0]	: left child
	 * c[1] : right child
	 * i	: bin slot
	 */
	struct chunk_t* p;
	struct chunk_t* c[2];
	size_t i;
};

/*
 * Arena data
 */
struct arena_t {
	void *start;
	void *end;

	struct chunk_t* lb[LB_COUNT];
	struct chunk_t* sb[SB_COUNT];

	struct chunk_t* cf;
};

static struct arena_t* arena;

/*
 * align size to upper double word
 */
static inline size_t mm_align4(size_t size) {
	size_t r = size & 0xFFFFFFFC;
	return ( r == size ? r : r +4);
}

/*
 * align size to upper quad word
 */
static inline size_t mm_align8(size_t size) {
	size_t r = size & 0xFFFFFFF8;
	return ( r == size ? r : r +8);
}

/*
 * Set chunk current size
 */
static inline void mm_set_csize(struct chunk_t* p,size_t size) {
	size &= 0xFFFFFFFC;
	p->csz = size | (p->csz & 0x00000003);
}

/*
 * Set chunk previous size
 */
static inline void mm_set_psize(struct chunk_t* p,size_t size) {
	size &= 0xFFFFFFFC;
	p->psz = size;
}

/*
 * Set chunk pbusy flag
 */
static inline void mm_set_pbusy(struct chunk_t* p) {
	p->csz |= PBUSY_BIT;
}

/*
 * Set chunk cbusy flag
 */
static inline void mm_set_cbusy(struct chunk_t* p) {
	p->csz |= CBUSY_BIT;
}

/*
 * Clear chunk pbusy flag
 */
static inline void mm_clr_pbusy(struct chunk_t* p) {
	p->csz &= PBUSY_MSK;
}

/*
 * Clear chunk cbusy flag
 */
static inline void mm_clr_cbusy(struct chunk_t* p) {
	p->csz &= CBUSY_MSK;
}

/*
 * Return chunk size
 */
static inline size_t mm_get_csize(struct chunk_t* p) {
	return p->csz & SIZE_MSK;
}

/*
 * Return previous chunk size
 */
static inline size_t mm_get_psize(struct chunk_t* p) {
	return p->psz;
}

/*
 * get current chunk busy bit
 */
static inline int mm_is_cbusy(struct chunk_t* p) {
	return (p->csz & CBUSY_BIT) == CBUSY_BIT;
}

/*
 * get previous chunk busy bit
 */
static inline int mm_is_pbusy(struct chunk_t* p) {
	return (p->csz & PBUSY_BIT) == PBUSY_BIT;
}

/*
 * get the fisrt chunk
 */
static inline struct chunk_t* mm_first() {
	return (struct chunk_t*)(arena->start - sizeof(size_t));
}

/*
 * get the last chunk
 */
static inline struct chunk_t* mm_last() {
	return (struct chunk_t*)(arena->end - 2*sizeof(size_t));
}

/*
 * get the next chunk
 */
static inline struct chunk_t* mm_next(struct chunk_t* p) {
	return (struct chunk_t*)((void*)p + mm_get_csize(p) + sizeof(size_t));
}

/*
 * get the prev chunk
 */
static inline struct chunk_t* mm_prev(struct chunk_t* p) {
	return (struct chunk_t*)((void*)p - mm_get_psize(p) - sizeof(size_t));
}

/*
 * Convert chunk to data payload
 */
static inline void* mm_chunk2data(struct chunk_t* p) {
	return (void*)p + 2*sizeof(size_t);
}

/*
 * Convert data to chunk
 */
static inline struct chunk_t* mm_data2chunk(void* p) {
	return (struct chunk_t*)(p - 2*sizeof(size_t));
}

/*
 * Add chunk to free list
 */
static inline void mm_link(struct chunk_t* p) {
	if (mm_get_csize(p) < MAX_SMALL) {
		mm_link_small(p);
	} else {
		mm_link_large(p);
	}
}

/*
 * Remove chunk from free list
 */
static inline void mm_unlink(struct chunk_t* p) {
	if (mm_get_csize(p) < MAX_SMALL) {
		mm_unlink_small(p);
	} else {
		mm_unlink_large(p);
	}
}

/*
 * debug chunk information
 */
void mm_debug() {
	struct chunk_t* p;

	for (p = mm_first() ; p!=mm_last(); p = mm_next(p)) {
		mm_debug_chunk(p);
	}
	mm_debug_chunk(p);
}

/*
 * Debug chunk information
 */
void mm_debug_chunk(struct chunk_t* p) {
	size_t flg = p->csz & FLAG_MSK;

	switch(flg) {
	case 0:
		printf("%#08x [ ][ ] %#08x %#08x\n", p, mm_get_psize(p) ,mm_get_csize(p));
		break;
	case 1:
		printf("%#08x [ ][P] ---------- %#08x\n",p ,mm_get_csize(p));
		break;
	case 2:
		printf("%#08x [C][ ] %#08x %#08x\n", p, mm_get_psize(p), mm_get_csize(p));
		break;
	case 3:
		printf("%#08x [C][P] ---------- %#08x\n",p ,mm_get_csize(p));
		break;
	}
}

/*
 * compute large slot 
 */
static size_t mm_slot_large(size_t v) {
        asm(
                "       mov %%eax,%%edx;        "
                "       shr $8,%%eax;           "
                "       bsr %%eax,%%ecx;        "
                "       mov %%ecx,%%eax;        "
                "       shl $2,%%eax;           "
                "       add $6,%%ecx;           "
                "       shr %%cl,%%edx;         "
                "       and $0x3, %%edx;        "
                "       add %%edx,%%eax;        "
                :"=a" (v)
                :"a" (v)
                : "ecx", "edx"
        );
	return v;	
}

/*
 * compute large key
 */
static size_t mm_key_large(size_t v) {
        asm(
                "       mov $34,%%cl;           "
                "       bsr %%eax,%%edx;        "
                "       sub %%dl,%%cl;          "
                "       shl %%cl,%%eax;         "
                : "=a" (v)
                : "a" (v)
                : "ecx", "edx"
        );
        return v;
}

/*
 * Memory Manager Initialization
 */
void mm_init() {
	struct chunk_t* s;
	struct chunk_t* e;

	MM_DEBUG("MM Init\n");

	arena = (struct arena_t*)MM_START;
	MM_DEBUG("arena base  : %#08x\n", arena);
	MM_DEBUG("arena size  : %#08x\n", sizeof(struct arena_t));

	arena->start = (void*)mm_align4((size_t)((void*)arena + sizeof(struct arena_t)));
	arena->end = (void*)MM_END;

	MM_DEBUG("arena start : %#08x\n",arena->start);
	MM_DEBUG("arena end   : %#08x\n",arena->end);

	/*
	 * Inital chunks 
	 */
	s = mm_first();
	e = mm_last();

	mm_set_csize(s, arena->end - arena->start - 2*sizeof(size_t));
	mm_set_pbusy(s);
	mm_clr_cbusy(s);

	mm_set_csize(e, 0x00000000);
	mm_set_psize(e, mm_get_csize(s));
	mm_clr_pbusy(e);
	mm_set_cbusy(e);

	mm_link(s);

	arena->cf = s;
}

/*
 * Allocate a chunk. Split it before if remaining space available
 * 
 * -The initial chunk is assumed to be large enough
 * -We split the initial chunk if there's enough space
 * 
 * Initial chunk   	: sini = 8s0
 * Allocated chunk 	: sreq = 8s1
 * Created chunk	: S
 *
 * 4 + 8s0 = 4 + 8s1 + 4 + S
 * S = 8(s0-s1) - 4
 *
 * we divide the chunk if and only if S>=16 
 * cad sini - sreq >= 20
 *
 *  ???? payload from previous chunk
 *  cccc curr size
 *  pppp prev size
 * 
 *       |....|        |....|
 * p0 -> +----+  p1 -> +----|
 *       |????|        |????|
 *       |cccc|        |cccc|
 *       |....|        |....|
 *       |....|  p2 -> +----+
 *       |....|        |????|
 *       |....|        |cccc|
 *       |....|        |....|
 * pn -> +----+  pn -> +----+
 *       |pppp|        |pppp|
 *       |cccc|        |cccc|
 *       |....|        |....|
 */
struct chunk_t* mm_split(struct chunk_t* p, size_t sreq) {
	size_t sini = mm_get_csize(p);

	struct chunk_t* n;

	if (sini - sreq >= 5 * sizeof(size_t)) {
		MM_DEBUG("Splitting chunk %#08x\n",p);
	
		mm_unlink(p);

		mm_set_csize(p,sreq);
		mm_set_cbusy(p);

		n = mm_next(p);	

		mm_set_csize(n , sini - sreq - sizeof(size_t));
		mm_set_pbusy(n);
		mm_clr_cbusy(n);

		mm_set_psize(mm_next(n), mm_get_csize(n));

		mm_link(n);

		return n;
	} else {
		// Too small to split

		mm_unlink(p);
		
		mm_set_cbusy(p);
		n = mm_next(p);
		mm_set_pbusy(n);

		return 0;
	}
}

/*
 * free chunk
 *
 * -coalesce this chunk with previous a next chunk if possible
 */
struct chunk_t* mm_coalesce(struct chunk_t* p) {
	size_t s;

	struct chunk_t* n;

	mm_clr_cbusy(p);

	n = mm_next(p);

	if (!mm_is_cbusy(n)) {
		// next chunk was free so next next chunk was busy
		// it will still be busy and we doesn't need to 
		// update its flags

		MM_DEBUG("Coalescing with next chunk\n");
		mm_unlink(n);
		s = mm_get_csize(n) + mm_get_csize(p) + sizeof(size_t);
		mm_set_csize(p, s);
		n = mm_next(p);
	}

	// Update previous chunk size of the next chunk	
	mm_set_psize(n, mm_get_csize(p));
	mm_clr_pbusy(n);

	if (!mm_is_pbusy(p)) {
		// previous wasn't busy

		MM_DEBUG("Coalescing with prev chunk\n");
		s = mm_get_psize(p) + mm_get_csize(p) + sizeof(size_t);
		p = mm_prev(p);

		mm_unlink(p);

		// just update size
		// flag doesn'change
		// new chunk still free and previous still busy
		mm_set_csize(p, s);
		mm_set_psize(mm_next(p), s);
	}

	// register it in the free list
	mm_link(p);

	return p;
}

/*
 * Add chunk to small free list
 */
void mm_link_small(struct chunk_t* p) {
	size_t s = mm_get_csize(p);
	size_t i = s >> 3;

	if (arena->sb[i] == 0) {
		p->prev = p;
		p->next = p;
		arena->sb[i] = p;
	} else {
		p->prev = arena->sb[i];
		p->next = p->prev->next;
		p->next->prev = p;
		p->prev->next = p;
	}
}

/*
 * Remove chunk from small free list
 */
void mm_unlink_small(struct chunk_t* p) {
	size_t s = mm_get_csize(p);
	size_t i = s >> 3;

	p->next->prev = p->prev;
	p->prev->next = p->next;

	if (p->next == p->prev) {
		p->next = 0;
		p->prev = 0;
	}

	if (arena->sb[i] == p) {
		arena->sb[i] = p->next;
	}
}

/*
 * Remove chunk from large free list
 */
void mm_unlink_large(struct chunk_t* p) {
	struct chunk_t* c =0;
	size_t j;

	MM_DEBUG("Unlinking %#08x\n",p);
	if (p->next != p) {
		// There's other chunk in ring list

		// Detaching p
		p->next->prev = p->prev;
		p->prev->next = p->next;

		if (p->p !=0 || p == arena->lb[p->i]) {
			//copy childs
			p->next->c[0] = p->c[0];
			p->next->c[1] = p->c[1];

			// update parent if needed
			if (p->p && p->p->c[0] == p) {
				p->p->c[0] = p->next;
			} else if (p->p && p->p->c[1] == p) {
				p->p->c[1] = p->next;
			}

			if (p == arena->lb[p->i]) {
				// This was the root, make the new root
				// be the next chunk on the ring
				arena->lb[p->i] = p->next;
				//p->next->p = 0;
			}
		} 
	} else {
		// We are in the tree
		// find the rightmost child
		c = p;
		
		for (;;) {
			j = c->c[1] != 0 ? 1 : 0;
			if (c->c[j] == 0) {
				break;
			}
		}	

		if (c == p) {
			// Chunk without child nodes
			if (p->p != 0) {
				// Update parent if needed
				j = p->p->c[0] == p ? 0 : 1;
				p->p->c[j]=0;
			}

			// And if we are the root, there isn't any root
			if (arena->lb[p->i] == p) {
				arena->lb[p->i] = 0;
			}
		} else {
			// We have child , c is the rightmost
			// and have no child
			// let's substitue p with c

			// update c parent
			// there is a parent otherwise we would have c == P
			j = c->p->c[0] == c ? 0 : 1;
			c->p->c[j] = 0;

			// replace c childs with p childs
			c->c[0] = p->c[0];
			c->c[1] = p->c[1];

			// if p has a parent, update the link to poiut c
			if (p->p != 0) {
				j = p->p->c[0] == p ? 0 : 1;
				p->p->c[j] = c;	
			}

			// finally if we were at the root
			// make c be the new root
			if (arena->lb[p->i] == p) {
				arena->lb[p->i] = c;
			}
		}
	}
}

/*
 * Add chunk to large free list
 */
void mm_link_large(struct chunk_t* n) {
	size_t s = mm_get_csize(n);
	size_t i = mm_slot_large(s);
	size_t k = mm_key_large(s);
	size_t j;
	struct chunk_t* p;

	MM_DEBUG("Linking chunk %#08x\n",n);

	n->p = 0;
	n->i = i;
	n->c[0] = 0;
	n->c[1] = 0;
	n->next = n;
	n->prev = n;

	if (arena->lb[i] == 0) {
		arena->lb[i] = n;
		n->p = 0;
	} else {
		p = arena->lb[i];
		
		for (;;) {
			if (s == mm_get_csize(p)) {
				// exact match
				// we put this chunk in ring list
				n->next = p->next;
				n->prev = p;
				p->next = n;
				n->next->prev = n;
				break;
			}

			j = (k >> 31);
			if (p->c[j]) {
				p = p->c[j];
				k<<=1;
			} else {
				p->c[j] = n;
				n->p = p;
				break;
			}
		}
	}
}

/*
 * find best fitting small chunk
 */
static struct chunk_t* mm_find_small(size_t s) {
	struct chunk_t* p = 0;

	size_t i = s >> 3;
	size_t j = 0;

	MM_DEBUG("Finding small chunk (%u) in slot %u\n",s,i);
	
	for (j = i; j< SB_COUNT; j++) {
		if (arena->sb[j]!=0) {
			p = arena->sb[j];
		}
	} 

	if (p == 0) {
		p = mm_find_large(0x100);
	}

	return p;
}

/*
 * find best fitting large chunk
 */
static struct chunk_t* mm_find_large(size_t s) {
	size_t m = 0xFFFFFFFF;
	size_t i = mm_slot_large(s);
	size_t k = mm_key_large(s);
	size_t w;
	size_t j;

	struct chunk_t* p = 0;
	struct chunk_t* c = 0;
	struct chunk_t* a = 0;

	if (arena->lb[i] != 0) {
		p = arena->lb[i];

		for (;;) {
			w = mm_get_csize(p);
			if (s <= w && w < m) {
				m = w;
				c = p;
				if (w == s) {
					break;
				}
			}
	
			j = k >> 31;
			
			if (j == 0 && p->c[1]!=0) {
				a = p->c[1];
			}

			p = p->c[j];
			k<<=1;

			if (p == 0) {
				p = a;
				break;
			}
		}

		while (p) {
			w = mm_get_csize(p);
			if (s <= w && w < m) {
				m = w;
				c = p;
			}
			p = p->c[p->c[0] != 0 ? 0 : 1];
		}
	}

	if (c == 0) {
		// Find chunk in remaining bins
		for (j = i+1 ; j < LB_COUNT; j++) {
			if (arena->lb[j] != 0) {
				p = arena->lb[j];

				while (p) {
					w = mm_get_csize(p);
					if (s <= w && w < m) {
						m = w;
						c = p;
					} 	
					p = p->c[p->c[0] != 0 ? 0 : 1];
				}
			}
		}
	}
	
	MM_DEBUG("Found Chunk %#08x\n",c);
	return c;
}

/*
 * malloc function
 */
void* mm_malloc(size_t size) {
	struct chunk_t *p;
	struct chunk_t *n;

	if (size < 4*sizeof(size_t)) {
		size = 4*sizeof(size_t);
	}

	// Align requested size on quad word boundary
	size = mm_align8(size);

	if (size >= MAX_SMALL) {
		p = mm_find_large(size);
	} else {
		p = mm_find_small(size);
	}
	
	if (p == 0) {
		return 0;
	}

	mm_split(p, size);

	return mm_chunk2data(p);
}

/*
 * free function
 */
void mm_free(void* ptr) {
	struct chunk_t* p = mm_data2chunk(ptr);

	p = mm_coalesce(p);
}

