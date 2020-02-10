/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Unix Standard functions
 */

#ifndef __LOADER_UTL_UNISTD_H__
#define __LOADER_UTL_UNISTD_H__

#include "loader/types.h"

size_t write(int fd,const void* buf,size_t count);

#endif

