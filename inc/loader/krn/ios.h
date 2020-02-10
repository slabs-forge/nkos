/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * IO Abstraction
 */

#ifndef __LOADER_KRN_IOS_H__
#define __LOADER_KRN_IOS_H__

#include "loader/types.h"

#define MAXDESC	10

struct ios_t;
struct iosm_t;

typedef size_t (*ios_write)(struct ios_t*,const void* buf, size_t count);

struct ios_t {
	void* d;
	struct iosm_t* m;
};

struct iosm_t {
	ios_write write;
};

typedef struct ios_t* (*ios_init)(int fd);

#endif

