/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 29/09/2011
 *
 * e820 region manipulation
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"
#include "kernel/e820.h"
#include "kernel/utl/kdebug.h"
#include "kernel/utl/string.h"

static void e820_debug(struct e820_t*);
static int64_t e820_count(struct e820_t*);
static void e820_sort(struct e820_t*,int64_t);
static void e820_overlap(struct e820_t*,int64_t*);
static void e820_coalesce(struct e820_t*,int64_t*);

/*
 * Compute end
 */
static inline uint64_t _mfin(struct e820_t* m) {
	return m->base + m->size;
}

#define _msta(m) (m->base)
#define _msiz(m) (m->size)
#define _mtyp(m) (m->type)

static void e820_debug(struct e820_t* m) {
	while (m->size) {
		kdebug("E820 mem area : %016lx - %016lx %d\n",m->base,m->base+m->size,m->type);
		m++;
	}
}

/*
 * Count entries
 */
static int64_t e820_count(struct e820_t* m) {
	int64_t c = 0;
	while (m++->size) c++;
	return c;	
}

/*
 * Sort entries
 *
 * - We use a insertion sort algorithm since 
 *   the array should already be sorted and
 *   it's simple
 * - Entries are sorted by growing adress, then by growing size
 */
static void e820_sort(struct e820_t* m,int64_t count) {
	struct e820_t t;

	for (int64_t i = 1 ; i < count; i ++) {
		int64_t j = i;

		memmove(&t,&m[i],sizeof(struct e820_t));

		while (j > 0 && (t.base < m[j-1].base || ( t.base == m[j-1].base && t.size < m[j-1].size))) {
			memmove(&m[j],&m[j-1],sizeof(struct e820_t));
			j--;
		}
		if (j != i) memmove(&m[j],&t,sizeof(struct e820_t));
	}
}

/*
 * Discard an entry
 */
void e820_discard(struct e820_t* m,int64_t i,int64_t* pc) {
	if (i +1 < *pc) {
		memmove(&m[i],&m[i+1],(*pc - i -1) * sizeof(struct e820_t));
	}
	memset(&m[*pc-1],0,sizeof(struct e820_t));

	*pc = *pc -1;
}

/*
 * Deal with overlap
 */
void e820_overlap(struct e820_t* m,int64_t* pc) {

	for (int64_t i = 0 ; i < *pc -1; i++) {
		struct e820_t* c = &m[i];
		struct e820_t* n = &m[i+1];

		/*
		 * Skip unused
		 */
		if (_mtyp(c) == 0) continue;

		/*
		 * Skip if no overlap
		 */
		if (_mfin(c) <= n->base) continue;

		if (_msta(n) == _msta(c) && _mfin(n) == _mfin(c)) {
			/*
			 * Same block adress, same length
			 * discard the block with lowest priority
			 * and re process same entry
			 */
			if (_mtyp(n) >= _mtyp(c)) {
				e820_discard(m,i,pc);
				i--;
			} else {
				e820_discard(m,i+1,pc);
				i--;
			}
			continue;
		}

		if (_msta(c) == _msta(n)) {
			/*
			 * Same base adress
			 * -Entries should have been sorted so next entries is bigger than this
			 */
			if (_mtyp(c) <= _mtyp(n)) {
				/*
				 * Discard current
				 */
				e820_discard(m,i,pc);
				i--;
				continue;
			}

			_msta(n) = _mfin(c);
			_msiz(n) -= _msiz(c);
			e820_sort(c,*pc - i );
			i--;
			continue;
		}

		if (_mfin(c) == _mfin(n)) {
			/*
			 * Same end address
			 */
			if (_mtyp(c) >= _mtyp(n)) {
				e820_discard(m,i+1,pc);
				i--;
				continue;
			}

			_msiz(c) = _msta(n) - _msta(c);
			continue;
		}

		if (_mfin(n) < _mfin(c)) {
			/*
			 * Next region is inside
			 */
			if (_mtyp(n) > _mtyp(c)) {
				/*
				 * In the middle, we need to create another region
				 */
				if (*pc < E820_ENTRIES) {
					/*
					 * There's enough room, let's create it at the end
					 * 11111 => 1	1
					 *  222  =>  222
					 */
					m[*pc].size = _msiz(c); 
					_msiz(c) = _msta(n) - _msta(c);
					m[*pc].base = _mfin(n);
					m[*pc].size = m[*pc].size - _msiz(c) - _msiz(n); 
					m[*pc].type = _mtyp(c);
					(*pc)++;
					e820_sort(n,*pc - i - 1);
				} else {
					/*
					 * Not enough room, let's forget the 3rd region
					 * 11111 => 1
					 *  222  =>  22222
					 */
					_msiz(n) = _msiz(c);
					_msiz(c) = _msta(n) - _msta(c);
					_msiz(n) -= _msiz(c);
				}
			} else {
				/*
				 * Lower priority region inside, discard it
				 */
				e820_discard(m,i+1,pc);
				i--;
				continue;
			}
		} else {
			/*
			 * next region goes beyond the first
			 */
			if (_mtyp(c) > _mtyp(n)) {
				/*
				 * 22222  => 22222
				 *  11111 =>      1
				 */
				_msiz(n) -= _mfin(c) - _msta(n);
				_msta(n) = _mfin(c);
				e820_sort(n, *pc - i - 1);					
			} else  {
				/*
				 * 11111  => 1
				 *  22222 =>  22222
				 */
				_msiz(c) = _msta(n) - _msta(c);
			}
		}
	}
}

/*
 * Coaelesce sibling entries
 */
void e820_coalesce(struct e820_t* m,int64_t* pc) {
	int64_t i;

	for (i = 0; i < *pc -1 ; i++) {
		struct e820_t* c = &m[i];
		struct e820_t* n = &m[i+1];

		if (_mfin(c) == _msta(n) && _mtyp(c) == _mtyp(n)) {
			_msiz(c)+=_msiz(n);
			e820_discard(m,i+1,pc);
			i--;
			continue;
		}
	}
}

/*
 * Initialization of e820 region
 */
void e820_init(struct bootparm_t* bp) {
	struct e820_t* m = bp->mem;

	int64_t count = e820_count(m);

	ASSERT(count < E820_ENTRIES-3);

	/*
	 * Add kernel to zone and 1st page
	 */
	m[count].base = 0x100000;
	m[count].size = bp->kernel_offset + (__bss - __text);
	m[count].type = 5;
	count++;
	m[count].base = 0x0000;
	m[count].size = 0x1000;
	m[count].type = 5;
	count++;

	/*
	 * Sort the entries by address,size
	 */
	e820_sort(m,count);

	/*
	 * Suppress overlapping region
	 */
	e820_overlap(m,&count);

	/*
	 * Coalesce neighbour zone with same type
	 */
	e820_coalesce(m,&count);

	e820_debug(m);
}

