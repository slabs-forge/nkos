/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 14/10/2011
 *
 * Physical Allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"
#include "kernel/bootparm.h"
#include "kernel/e820.h"

#include "kernel/mm/zone.h"

#include "kernel/utl/kdebug.h"

/*
 * Static data
 */
static uint32_t _zcnt __attribute__((section(".data"))) = 0;
static struct zone_t _zones[MAX_ZONES] __attribute__((section(".data")));

/*
 * Prototypes
 */
static void _zone_add(uint64_t bgn,uint64_t end,uint64_t flags,const char* name);

/*
 * Zone Init
 */
void zone_init() {
	uint64_t maxaddr = 0x0;

	struct e820_t* p = kbootparm()->mem;

	for (uint16_t i = 0; i < E820_ENTRIES; i++) {
		if (p[i].size == 0) break;
		if (p[i].type != 1) continue;

		if (p[i].base + p[i].size > maxaddr) maxaddr = p[i].base + p[i].size;
	}
	
	if (maxaddr > 0x00000000ffffffffULL) {
		/*
		 * Above 4go
		 */
		_zone_add(0x0000000100000000ULL,maxaddr,0,"High");
		_zone_add(0x0000000001000000ULL,0x00000000ffffffff,ZONE_DMA32,"DMA32");
		_zone_add(0x0000000000000000ULL,0x0000000000ffffff,ZONE_DMA32|ZONE_DMA,"DMA");
	} else if (maxaddr > 0x0000000001000000ULL) {
		/* 
		 * Above 16Mo
		 */
		_zone_add(0x0000000001000000ULL,maxaddr,ZONE_DMA32,"DMA32");
		_zone_add(0x0000000000000000ULL,0x0000000000ffffff,ZONE_DMA32|ZONE_DMA ,"DMA");
	} else {
		_zone_add(0x0000000000000000ULL,maxaddr,ZONE_DMA,"DMA");
	}

	zone_debug();
}

/*
 * Zone debug
 */
void zone_debug() {
	for (uint64_t i = 0 ; i < _zcnt ; i++) {
		kdebug("Zone: %016lx-%016lx (",_zones[i].bgn,_zones[i].end);
		if (_zones[i].flags & ZONE_DMA32) {
			kdebug("DMA32 ");
		}
		if (_zones[i].flags & ZONE_DMA) {
			kdebug("DMA ");
		}
		kdebug(") %s\n",_zones[i].name);
	}
}

/*
 * Add a zone to zone list
 */
static void _zone_add(uint64_t bgn,uint64_t end,uint64_t flags,const char* name) {
	ASSERT(_zcnt < MAX_ZONES);
	
	_zones[_zcnt].bgn = bgn;
	_zones[_zcnt].end = end;
	_zones[_zcnt].name = name;
	_zones[_zcnt].flags = flags;
	_zcnt++;
}


