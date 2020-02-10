/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Memory Manager functions
 *
 */

#ifndef __LOADER_KRN_MM_H__
#define __LOADER_KRN_MM_H__

void mm_debug();

void* mm_malloc(size_t size);
void mm_free(void*);

#endif

