/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Device Manager Includes
 *
 */

#ifndef __LOADER_KRN_DM_H__
#define __LOADER_KRN_DM_H__

#include "loader/types.h"
#include "loader/krn/dm_path.h"

#define LBA_H(x) ((uint32_t)(x>>32))
#define LBA_L(x) ((uint32_t)(x&0xFFFFFFFFULL))

#define DM_FLAG_FS	0x00000001

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/*
 * Partition entry
 */
struct dm_partition_t {
        uint8_t active;
        uint8_t hs;
        uint8_t ss;
        uint8_t cs;
        uint8_t type;
        uint8_t he;
        uint8_t se;
        uint8_t ce;
        uint32_t lba;
        uint32_t size;
};

struct dm_dev_t;
struct dm_file_t;

struct dm_dev_method_t {
	int32_t (*init)(struct dm_dev_t*);
	struct dm_file_t* (*open)(struct dm_dev_t*,struct dm_path_t*);
	int32_t (*read)(struct dm_dev_t*,uint64_t,uint32_t,uint32_t,void*);
	uint32_t (*blocksize)(struct dm_dev_t*);
	uuid_t (*get_uuid)(struct dm_dev_t*);
};

struct dm_dev_t {

	void* data;
	struct dm_dev_method_t* meth;	

	const char* name;
	uint32_t flags;
	struct dm_dev_t* next_dev;
	struct dm_dev_t* next_mnt;
	struct dm_dev_t* prev_dev;
	struct dm_dev_t* prev_mnt;
};

struct dm_file_method_t {
	uint64_t (*read)(struct dm_file_t*,void*,uint64_t);
	int32_t (*close)(struct dm_file_t*);
	int32_t (*llseek)(struct dm_file_t*,uint64_t,uint64_t*,int);
};

struct dm_file_t {
	struct dm_dev_t* dev;
	void* data;
	struct dm_file_method_t* meth;
	uint64_t offset;
};

struct dm_mod_method_t {
	struct dm_dev_t* (*create_device)(struct dm_dev_t* parent, uint32_t i, uint64_t, uint64_t);
};

struct dm_module_t {
	uint8_t type;
	const char* name;
	struct dm_module_t* next; 
	struct dm_mod_method_t* meth;
};

int32_t dm_init(struct dm_dev_t*);
int32_t dm_read(struct dm_dev_t*,uint64_t,uint32_t,uint32_t,void*);
uint32_t dm_blocksize(struct dm_dev_t*);
struct dm_module_t* dm_find_module(uint8_t);

struct dm_dev_t* dm_find_mnt_by_name(const char*,size_t);
struct dm_dev_t* dm_find_mnt_by_uuid(uuid_t);

const char* dm_mk_name(const char*,uint16_t);

int32_t dm_mount_device(struct dm_dev_t*);
int32_t dm_register_device(struct dm_dev_t*);

void dm_register_module(struct dm_module_t*);

struct dm_file_t* dm_file_open(const char *fname);
uint64_t dm_file_read(struct dm_file_t*,void* buffer,uint64_t size);
int32_t dm_file_close(struct dm_file_t*);
int32_t dm_file_llseek(struct dm_file_t*,uint64_t,uint64_t*,int);

#define DECLARE_DM_MODULE(NAME,TYPE,METHODS) \
static struct dm_module_t mod_info = { .type = TYPE, .name = NAME, .meth = &METHODS }; \
static void mod_init() __attribute__((constructor (30001))); \
static void mod_init() { \
dm_register_module(&mod_info); \
}

#endif

