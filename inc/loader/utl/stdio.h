/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Standard IO
 */

#ifndef __LOADER_UTL_STDIO_H__
#define __LOADER_UTL_STDIO_H__

typedef void FILE;

#include "loader/stdarg.h"

void* stdout;

void vfprintf(FILE*, const char* format,va_list ap);
void printf(const char* format,...);

#endif

