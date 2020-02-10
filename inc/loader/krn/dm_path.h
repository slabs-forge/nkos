/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 14/09/2011
 *
 * Device Manager path utilities
 *
 */

#ifndef __LOADER_KRN_DM_PATH__
#define __LOADER_KRN_DM_PATH__

#include "loader/types.h"

struct dm_path_t;

struct dm_path_t* dm_path_create(const char*);
struct dm_path_t* dm_path_prepend(struct dm_path_t*,const char*);
void dm_free(struct dm_path_t*);

const char* dm_path_name(struct dm_path_t*);
size_t dm_path_size(struct dm_path_t*);

void dm_path_next(struct dm_path_t*);
void dm_path_debug(struct dm_path_t*);

#endif

