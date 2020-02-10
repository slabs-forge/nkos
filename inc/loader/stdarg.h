/*
 * NK Loader - Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Variadic support
 */

#ifndef __LOADER_STDARG_H__
#define __LOADER_STDARG_H__ 

typedef void* va_list;

#define va_start(ap,last) { ap = ((void*) &last) + sizeof(last); }
#define va_end(ap) { ap = 0; }

static inline void* _va_arg(va_list* ap,uint32_t len) {
	void *r=*ap;
	*ap+=(len<4 ? 4 : len);
	return r;
}

#define va_arg(ap,type) *((type*)_va_arg(&ap,sizeof(type)))

#endif


