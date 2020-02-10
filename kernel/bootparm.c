/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 26/09/2011
 *
 * Boot Parameter
 */

#include "kernel/types.h"
#include "kernel/serial.h"
#include "kernel/kernel.h"
#include "kernel/bootparm.h"

/*
 * Copy of bootparm
 * leave this in .data so that we can make a copy early
 */
struct bootparm_t g_bootparm __attribute__((section(".data")));

struct bootparm_t* kbootparm(void) {
	return &g_bootparm;
}
