/*
 * NK Loader - Kernel Functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * IO Abstraction
 */

#include "loader/types.h"
#include "loader/errno.h"
#include "loader/krn/ios.h"

struct ios_t* ios_tm_init();

struct ios_ent_t {
	struct ios_t* data;
	ios_init init;
};

static struct ios_ent_t ios_desc[MAXDESC] = {
	{ .init = ios_tm_init }
};

struct ios_t* ios_get(int fd) {
	if (fd > MAXDESC) {
		errno = ENODEV;
		return 0;
	}

	if (ios_desc[fd].data != 0) {
		return ios_desc[fd].data;
	}

	if (ios_desc[fd].init != 0) {
		ios_desc[fd].data = ios_desc[fd].init(fd);
		ios_desc[fd].init = 0;
		return ios_desc[fd].data;
	}

	return 0;
}

size_t write(int fd, const void* buf, size_t count) {
	struct ios_t* ios = ios_get(fd);		
	
	if (ios == 0) {
		errno = EINVAL;
		return -1;
	}

	if (ios->m->write == 0) {
		errno = ENOSYS;
		return -1;
	}

	return ios->m->write(ios,buf,count);
}
