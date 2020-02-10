/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 26/09/2011
 *
 * cpu info
 */

#include "kernel/types.h"
#include "kernel/cpuinfo.h"

#include "kernel/utl/kdebug.h"
#include "kernel/utl/cpuid.h"

/*
 * Structure definition
 */
struct cpu_cache_info_t {
	/*
	 * Cache line size
	 */
	uint16_t line;
};

/*
 * Static info
 */
static struct cpu_cache_info_t _caches[CPU_CACHE_LEVEL_MAX*CPU_CACHE_TYPE_MAX] = {
	{ .line = 32 },
	{ .line = 32 },
	{ .line = 32 },
	{ .line = 32 },
	{ .line = 32 },
	{ .line = 32 },
};

/*
 * Initialize cpuinfo structures
 */
void cpuinfo_init(void) {
	static const char* _cache_type[] = {
		0,
		"data",
		"code",
		"unified",
	};

	struct cpuid_t info;
	uint32_t maxcpuid;

	/*
	 * Getinf max cpuid
	 */
	info.eax = 0x00000000;
	cpuid(&info);
	maxcpuid = info.eax;

	/*
	 * Read cache info
	 */
	if (maxcpuid >= 0x00000004) {
		for (uint32_t i = 0 ; ; i++) {
			uint8_t lvl;
			uint8_t typ;
			uint16_t line;
			uint16_t part;
			uint32_t ways;
			uint32_t sets;
			
			info.eax = 0x00000004;
			info.ecx = i;
	
			cpuid(&info);

			if ((info.eax & 0x1f) == 0x00) break;
		
			lvl = (uint8_t)((info.eax >> 5) & 0x7) - 1;
			typ = (uint8_t)(info.eax & 0x3);
			line = (uint16_t)(info.ebx & 0x7FF) + 1;
			part = (uint16_t)((info.ebx >> 12) & 0x3ff) + 1;
			ways = (uint32_t)(info.ebx >> 22) + 1;
			sets = info.ecx + 1;

			kdebug("L%d %dkb", lvl + 1,(ways*part*sets*line) >> 10);
			kdebug(" %s %d-ways cache", _cache_type[typ],ways);
			kdebug(" (size = %d, part = %d",line ,part);
			kdebug(",sets = %d)",sets);
			kdebug("\n");

			if (lvl < CPU_CACHE_LEVEL_MAX) { 
				if ((typ & 0x1) == 0x1) {
					_caches[lvl << 1].line = line;
				}

				if ((typ & 0x2) == 0x2) {
					_caches[(lvl << 1) + 1].line = line;
				}
			}
		}
	}
}	

/*
 * Get Cache Line Size
 */
uint64_t cpuinfo_get_cache_line(uint8_t lvl,uint8_t typ) {
	if (lvl >= CPU_CACHE_LEVEL_MAX) return 0;
	if (typ >= CPU_CACHE_TYPE_MAX) return 0;

	return _caches[(lvl << 1) + typ].line;
}

