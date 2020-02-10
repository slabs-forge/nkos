/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * error handling
 */

#ifndef __LOADER_ERRNO_H__
#define __LOADER_ERRNO_H__

#include "loader/types.h"

#define EPERM	1
#define ENOENT	2
#define EIO	5
#define ENOMEM	12
#define ENODEV	19
#define EISDIR	21
#define EINVAL	22
#define ENOSYS	38
#define ELOOP	40

extern uint32_t errno;

#if defined(DEBUG)
#define DEBUG(FORMAT,...) printf(FORMAT, ## __VA_ARGS__)
#else
#define DEBUG(FORMAT,...) 
#endif

void debug(const char* format, ...);
void error(const char* format, ...);
void panic(const char* format, ...);

#endif
