/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * Loader utility function
 */

#ifndef __LOADER_LOADER_H__
#define __LOADER_LOADER_H__

struct bootparm_t {
	char kernel_image[128];
	void* kernel_address;
};

void halt() __attribute__((noreturn));
void jump() __attribute__((noreturn));

#endif
