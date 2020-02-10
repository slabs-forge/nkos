/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * UUID functions
 */

#ifndef __LOADER_UTL_UUID_H__
#define __LOADER_UTL_UUID_H__

#include "loader/types.h"

void uuid_unparse(uuid_t in,char *out);
int32_t uuid_parse(const char* in,uuid_t uu);
int32_t uuid_compare(uuid_t uu1,uuid_t uu2);
 
#endif
