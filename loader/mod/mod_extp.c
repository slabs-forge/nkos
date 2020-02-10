/*
 * NK Loader - Modules
 *
 * Author       : Sebastien LAURENT
 * Date         : 07/09/2011
 *
 * Extended partition pseudo filesystem
 */

#include "loader/errno.h"

#include "loader/utl/stdlib.h"

#include "loader/krn/dm.h"

struct dm_extp_data_t {
	struct dm_dev_t* parent;
	uint64_t block_offset;
	uint64_t block_count;
};

static struct dm_dev_t* mod_create_device(struct dm_dev_t*, uint32_t, uint64_t offset, uint64_t size);

static int32_t dm_extp_init(struct dm_dev_t*);
static int32_t dm_extp_read(struct dm_dev_t*, uint64_t,uint32_t,uint32_t,void*);
static uint32_t dm_extp_blocksize(struct dm_dev_t*);

static struct dm_dev_method_t dm_extp_method = {
        .init = dm_extp_init,
        .read = dm_extp_read,
	.blocksize = dm_extp_blocksize
};

static struct dm_mod_method_t mod_method = {
        .create_device = mod_create_device
};

DECLARE_DM_MODULE("extp",0x05,mod_method);

/*
 * Initialisation d'une partition
 */
static struct dm_dev_t* mod_create_device(struct dm_dev_t* parent_device, uint32_t num, uint64_t offset, uint64_t size) {
	struct dm_dev_t *p;
	struct dm_extp_data_t* d;

	p = (struct dm_dev_t*) malloc(sizeof(struct dm_dev_t));
	d = (struct dm_extp_data_t*) malloc(sizeof(struct dm_extp_data_t));

	if (p == 0 || d == 0) {
		errno = ENOMEM;		
		if (p != 0) free(p);
		if (d != 0) free(p);

		return 0;
	}

	memset(p,0, sizeof(struct dm_dev_t));
	memset(d,0, sizeof(struct dm_extp_data_t));

	d->parent = parent_device;
	d->block_offset = offset;
	d->block_count = size;

	p->data = d;
	p->meth = &dm_extp_method;

        return p;
}

/*
 * Device Init
 */
static int32_t dm_extp_init(struct dm_dev_t* dev) {
	uint32_t offset;
	uint32_t bs;
	int32_t cr;
	uint32_t i = 5;

	struct dm_dev_t* sub;
	struct dm_module_t* mod;
	struct dm_partition_t* p;
	struct dm_extp_data_t* d = (struct dm_extp_data_t*) dev->data;

	p = (struct dm_partition_t*) malloc(2*sizeof(struct dm_partition_t));
	if (p == 0) {
		errno = ENOMEM;
		return -1;
	}

	for (offset = 0;;i++) {
		cr = dm_read(dev, offset,0x1be , 2*sizeof(struct dm_partition_t), p);
		if (cr<0) {
			break;
		}

		mod = dm_find_module(p[0].type);

		if (mod != 0 && mod->meth != 0 && mod->meth->create_device != 0) {
			sub = mod->meth->create_device(dev, i, offset+p[0].lba, p[0].size);

			if (sub !=0) {
				sub->name = dm_mk_name(d->parent->name,i);
				if (dm_init(sub) == 0) {
					dm_register_device(sub);
				}
			}
		}
	
		offset = offset + p[1].lba;
		if (p[1].lba == 0) {
			break;
		}
	}

	free(p);

	return 0;
}

/*
 * Device read
 */
static int32_t dm_extp_read(struct dm_dev_t* dev, uint64_t sect,uint32_t offset, uint32_t size,void* buffer) {
	struct dm_extp_data_t* d = dev->data;
	
	if (sect + ( offset + size) / dm_blocksize(dev) >= d->block_count) {
		errno = EIO;
		return -1;
	}

	return dm_read(d->parent,sect + d->block_offset, offset, size, buffer);
}

/*
 * Get blocksize
 */
static uint32_t dm_extp_blocksize(struct dm_dev_t* dev) {
	struct dm_extp_data_t* d = dev->data;
	return dm_blocksize(d->parent);
}
