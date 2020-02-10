/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Kernel main
 */

#include "loader/types.h"
#include "loader/errno.h"
#include "loader/loader.h"

#include "loader/utl/stdio.h"
#include "loader/utl/string.h"
#include "loader/utl/stdlib.h"

#include "loader/krn/dm.h"

void main(struct bootparm_t* bp) {
	int32_t cr;

	uint64_t offset;
	uint64_t read;

	struct dm_file_t* f;

	void* buf = (void*)0x100000;

	printf("NKLoader init done\n");	
	printf("kernel image   | %s\n",bp->kernel_image);
	printf("kernel address | %#08x\n",bp->kernel_address);

	f = dm_file_open(bp->kernel_image);
	if (f == 0) {
		error("can't open kernel image file - errno = %d\n",errno);
		halt();
	}

	cr = dm_file_llseek(f, 0 , &offset, SEEK_END);
	if (cr < 0) {
		error("can't read kernel image file - errno = %d\n",errno);
		halt();
	}

	printf("kernel size    | %u\n", (uint32_t) (offset & 0xFFFFFFFF));
	
	cr = dm_file_llseek(f, 0, 0, SEEK_SET);
	if (cr < 0) {
		error("can't read kernel image file - errno = %d\n",errno);
		halt();
	}
	
	printf("Loading kernel...");

	read = dm_file_read(f, buf, offset);
	if (read != offset) {
		error("can't read kernel image file - errno = %d\n",errno);
		halt();
	}

	dm_file_close(f);

	printf("done\n");

	printf("Jumping to kernel code\n");

	jump();
}
