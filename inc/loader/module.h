/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Modules
 */

#ifndef __LOADER_MODULE_H__
#define __LOADER_MODULE_H__

#define DECLARE_BUILTIN_MODULE(NAME,INIT,PRIO) \
void INIT() __attribute__((constructor (PRIO))); 

#endif

