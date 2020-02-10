/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Device Manager (Disk)
 */

#include "loader/module.h"

#include "loader/krn/dm.h"
#include "loader/krn/dm_path.h"

#include "loader/utl/stdlib.h"
#include "loader/utl/string.h"
#include "loader/utl/conv.h"
#include "loader/utl/uuid.h"

#include "loader/bs/bios13.h"

#include "loader/errno.h"

DECLARE_BUILTIN_MODULE("disk",dm_doinit,30000);

/*
 * data for raw devices
 */
struct dm_dev_raw_data_t {
	uint8_t device_id;
	uint64_t block_count;
	uint32_t block_size;
};

/*
 * Prototypes
 */
int32_t dm_init(struct dm_dev_t*);
int32_t dm_read(struct dm_dev_t*, uint64_t,uint32_t,uint32_t,void*); 
uint32_t dm_blocksize(struct dm_dev_t*);
uuid_t dm_get_uuid(struct dm_dev_t*);

struct dm_module_t* dm_find_module(uint8_t);

static struct dm_dev_t* dm_raw_create(uint8_t,uint64_t,uint32_t);
static int32_t dm_raw_init(struct dm_dev_t*);
static int32_t dm_raw_read(struct dm_dev_t*, uint64_t,uint32_t,uint32_t,void*);
static uint32_t dm_raw_blocksize(struct dm_dev_t*);
static void dm_mount_all();
/*
 * Linked list of dm modules
 */
struct dm_module_t* dm_mods;

/*
 * raw device method
 */
struct dm_dev_method_t dm_raw_method = {
	.init = dm_raw_init,
	.read = dm_raw_read,
	.blocksize = dm_raw_blocksize
};

static struct dm_dev_t* dm_reg_dev = 0;
static struct dm_dev_t* dm_mnt_dev = 0;

/*
 * Initialisation
 */
void dm_doinit() {
	struct dm_dev_t* dev;
	struct bs13_drive_parms_t parms;

	uint8_t i;
	int32_t cr;

	DEBUG("DM Init\n");

	for (i = 0x80; i < 0x84 ; i++) {
		cr = bs13_get_drive_parms(i, &parms);
		if (cr == 0) {
			dev = dm_raw_create(i, parms.lba, parms.bps);		
			if (dev != 0) {
				dm_init(dev);
			}
		}
	}

	dm_mount_all();
}

/*
 * Enregistrement d'un module DM
 */
void dm_register_module(struct dm_module_t* p) {
	DEBUG("Registering DM Module %s for partition type %2x\n",p->name,p->type);

	p->next = dm_mods;
	dm_mods = p;
}

/*
 * Recherche module par type
 */
struct dm_module_t* dm_find_module(uint8_t type) {
	struct dm_module_t* p;

	for (p = dm_mods; p!=0; p = p->next) {
		if (p->type == type) {
			return p;
		}
	}
	return 0;
}

/*
 *	Device initialisation
 */
int32_t dm_init(struct dm_dev_t* dev) {
	if (dev->meth && dev->meth->init) {
		return dev->meth->init(dev);
	} else {
		errno = ENOSYS;
		return -1;
	}
}

/*
 * 	Read sectors;
 */
int32_t dm_read(struct dm_dev_t* dev,uint64_t sect,uint32_t offset,uint32_t size,void* buf) {
	if (dev->meth && dev->meth->read) {
		return dev->meth->read(dev,sect,offset,size,buf);
	} else {
		errno = ENOSYS;
		return -1;
	}
}

/*
 * 	Get blocksize
 */
uint32_t dm_blocksize(struct dm_dev_t* dev) {
	if (dev->meth && dev->meth->blocksize) {
		return dev->meth->blocksize(dev);
	} else {
		errno = ENOSYS;
		return 0;
	}
}

/*
 * Get device uuid
 */
uuid_t dm_get_uuid(struct dm_dev_t* dev) {
	if (dev->meth && dev->meth->get_uuid) {
		return dev->meth->get_uuid(dev);
	} else {
		errno = ENOSYS;
		return 0;
	}
}

struct dm_dev_t* dm_raw_create(uint8_t device_id,uint64_t block_count,uint32_t block_size) {
	struct dm_dev_t* dev;
	struct dm_dev_raw_data_t *data;

	data = (struct dm_dev_raw_data_t*) malloc(sizeof(struct dm_dev_raw_data_t));

	if (data == 0) {
		errno = ENOMEM;
		return 0;
	}

	dev = (struct dm_dev_t*) malloc(sizeof(struct dm_dev_t));
	if (dev == 0) {
		errno = ENOMEM;
		free(data);
		return 0;
	}

	memset(data,0,sizeof(struct dm_dev_raw_data_t));
	memset(dev,0,sizeof(struct dm_dev_t));

	dev->name = dm_mk_name(0,device_id & 0x7f);
	dev->data = data;
	dev->meth = &dm_raw_method;

	data->device_id = device_id;
	data->block_count = block_count;
	data->block_size = block_size;

	return dev;
}

/*
 *	Raw Device Init
 */
static int32_t dm_raw_init(struct dm_dev_t* dev) {
	uint8_t i;
	int32_t cr;

	struct dm_dev_raw_data_t* d = (struct dm_dev_raw_data_t*) dev->data;
	struct dm_dev_method_t* m = dev->meth;

	struct dm_dev_t* sub;
	struct dm_module_t* mod;
	struct dm_partition_t* p;

	p = (struct dm_partition_t*) malloc(4*sizeof(struct dm_partition_t));
	if (p == 0) {
		errno = ENOMEM;
		return -1;
	}

	cr = dm_read(dev,0x0ULL,0x1be, 4*sizeof(struct dm_partition_t),p);
	if (cr == 0) {
		for (i = 0 ; i < 4; i++) {
			mod = dm_find_module(p[i].type);
		
			if (mod != 0 && mod->meth !=0 && mod->meth->create_device !=0) {
				sub = mod->meth->create_device(dev, i, p[i].lba, p[i].size);
				if (sub != 0) {
					sub->name = dm_mk_name(dev->name,i);

					if (dm_init(sub) == 0) {
						dm_register_device(sub);
					}
				}
			}
		}
	}

	free(p);

	return 0;
}

/*
 * Lecture Raw Device
 */
static int32_t dm_raw_read(struct dm_dev_t* dev, uint64_t sect,uint32_t offset,uint32_t size,void* buffer) {
	uint8_t* buf;
	uint32_t cr = 0;
	uint32_t sz;
	uint32_t bs;
	uint64_t ls;
	uint64_t cs;

	struct dm_dev_raw_data_t* d = (struct dm_dev_raw_data_t*) dev->data;

	bs = dm_blocksize(dev);
	cs = sect + offset / bs;
	ls = sect + (offset + size) / bs;

	
	// check that we are not past the end
	if (ls >= d->block_count) {
		errno = EIO;
		return -1;
	}

	// Temporary buffer allocation
	buf = (uint8_t*) malloc(bs);
	if (buf == 0) {
		errno = ENOMEM;
		return -1;
	}

	// modulo block size for the 1st round
	offset%=bs;
	while (size > 0) {
		// Read one sect
		cr = bs13_read_sect(d->device_id,cs, 1, buf);
		if (cr != 0) {
			errno = EIO;
			break;
		}

		// moving byte to final buffer
		sz = size < bs-offset ? size : bs-offset;
		//debug("read(%08x%08x,%08x,%08x)\n",LBA_H(cs),LBA_L(cs),sz,buffer);
		memmove(buffer,buf+offset, sz);
		buffer+=sz;

		size-= sz;
		offset = 0;
		cs++;
	}
	
	free(buf);

	return cr;
}

/*
 * Renvoie la taille de block
 */
static uint32_t dm_raw_blocksize(struct dm_dev_t* dev) {
	struct dm_dev_raw_data_t* d = (struct dm_dev_raw_data_t*) dev->data;
	return d->block_size;
}

/*
 * file read
 */
uint64_t dm_file_read(struct dm_file_t* file, void* buffer, uint64_t size) {
	if (file && file->meth && file->meth->read) {
		return file->meth->read(file,buffer,size);
	} else {
		errno = ENOSYS;
		return 0;
	}
}

/*
 * Build device name
 */
const char* dm_mk_name(const char* name,uint16_t num) {
	char tmp[8];
	char buf[32];
	const char *t;

	t = conv_uint16_d(num,tmp,sizeof(tmp));

	strcpy(buf, name != 0 ? name : "hd");

	if (name != 0) {
		strcat(buf,"p");
	}

	strcat(buf,t);
	return strdup(buf);	
}

/*
 * Registering the device
 */
int32_t dm_register_device(struct dm_dev_t* dev) {
	if (dm_reg_dev == 0) {
		dm_reg_dev = dev;
		dev->next_dev = dev;
		dev->prev_dev = dev;
	} else {
		dev->next_dev = dm_reg_dev;
		dev->prev_dev = dev->next_dev->prev_dev;

		dev->prev_dev->next_dev = dev;
		dev->next_dev->prev_dev = dev;
	}
	return 0;
}

/*
 * Mounting the device
 */
int32_t dm_mount_device(struct dm_dev_t* dev) {
	if ( (dev->flags & DM_FLAG_FS) != DM_FLAG_FS) {
		return -1;
	}

	DEBUG("Mounting device %s\n", dev->name);

	if (dm_mnt_dev == 0) {
		dm_mnt_dev = dev;
		dev->next_mnt = dev;
		dev->prev_mnt = dev;
	} else {
		dev->next_mnt = dm_mnt_dev;
		dev->prev_mnt = dev->next_mnt->prev_mnt;

		dev->prev_mnt->next_mnt = dev;
		dev->next_mnt->prev_mnt = dev;
	}

	return 0;
}

/*
 * Find device by name
 */
struct dm_dev_t* dm_find_mnt_by_name(const char* name, size_t size) {
	struct dm_dev_t* dev = dm_mnt_dev;

	if (dev == 0) 
		return 0;

	for (;;) {

		if (dev->name != 0 && strncmp(dev->name, name,size) == 0) return dev;

		dev = dev->next_mnt;

		if (dev == dm_mnt_dev) break;
	}
	return 0;
}

/*
 * Find device by uuid
 */
struct dm_dev_t* dm_find_mnt_by_uuid(uuid_t uuid) {
	struct dm_dev_t* dev = dm_mnt_dev;
	uuid_t uuid_dev;

	if (dev == 0) 
		return 0;

	for (;;) {
		uuid_dev = dm_get_uuid(dev);
		if (uuid_dev != 0 && uuid_compare(uuid_dev,uuid) == 0) {
			return dev;
		}
		
		dev = dev->next_mnt;

		if (dev == dm_mnt_dev) break;
	}

	return 0;
}

/*
 * Mount all mountable device
 */
void dm_mount_all() {
	struct dm_dev_t* dev = dm_reg_dev;

	if (dev == 0) return;

	for(;;) {
		dm_mount_device(dev);
		
		dev = dev->next_dev;

		if (dev == dm_reg_dev) break;
	}
}

/*
 * Close file 
 */
int32_t dm_file_close(struct dm_file_t* file) {
	if (file && file->meth && file->meth->close) {
		return file->meth->close(file);
	} else {
		errno = ENOSYS;
		return -1;
	}
}

/*
 * llseek file
 */
int32_t dm_file_llseek(struct dm_file_t* file,uint64_t offset,uint64_t* result,int whence) {
	if (file && file->meth && file->meth->llseek) {
		return file->meth->llseek(file, offset, result, whence);
	} else {
		errno = ENOSYS;
		return -1;
	}
}

/*
 * Open file
 * 
 * format : 	<device name>/<path>
 *		{device uuid>}/<path>
 */
struct dm_file_t* dm_file_open(const char* filename) {
	struct dm_dev_t* dev = 0;
	struct dm_file_t* file = 0;
	struct dm_path_t* path;

	uint8_t uuid[16];

	const char* s;
	size_t n;

	int32_t cr;

	// the path must begin with device reference
	if (*filename == '/') {
		errno = ENODEV;
		return file;
	}

	path = dm_path_create(filename);
	DEBUG("device=%.*s\n",dm_path_size(path),dm_path_name(path));

	n = dm_path_size(path);
	s = dm_path_name(path);

	if (s[0] == '{') {
		if (n !=38 && s[37] != '}') {
			errno = ENODEV;
			goto failed; 
		}

		if (uuid_parse(s+1,uuid) != 0) {
			errno = ENODEV;
			goto failed; 
		}

		DEBUG("Finding device by uuid\n");
		dev = dm_find_mnt_by_uuid(uuid);
	} else {
		DEBUG("Finding device by name\n");
		dev = dm_find_mnt_by_name(s,n);
	}
	
	if (dev == 0) {
		errno = ENODEV;
		goto failed;
	}

	if (dev->meth && dev->meth->open) {
		file = dev->meth->open(dev,path);
	}
failed:
	dm_path_free(path);
	return file;
}

