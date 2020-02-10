/*
 * NK Loader - Modules
 *
 * Author       : Sebastien LAURENT
 * Date         : 06/09/2011
 *
 * ext2 filesystem
 */


#include "loader/errno.h"

#include "loader/utl/uuid.h"

#include "loader/krn/dm.h"

#define SUPERBLOCK_SIZE 1024

#define EXT2_SUPER_MAGIC 0xEF53

#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000
#define EXT2_S_FLAGS	0xF000
 
struct dm_ext2_dent_t {
	uint32_t inode;
	uint16_t length;
	uint8_t namelength;
	uint8_t type;
};

struct dm_ext2_bdt_t {
	uint32_t bg_block_bitmap;
	uint32_t bg_inode_bitmap;
	uint32_t bg_inode_table;
	uint16_t bg_free_blocks_count;
	uint16_t bg_free_inodes_count;
	uint16_t bg_used_dirs_count;
	uint16_t bg_pad;
	uint8_t bg_reserved[12];
};

struct dm_ext2_inode_t {
	uint16_t i_mode;
	uint16_t i_uid;
	uint32_t i_size;
	uint32_t i_atime;
	uint32_t i_ctime;
	uint32_t i_mtime;
	uint32_t i_dtime;
	uint16_t i_gid;
	uint16_t i_links_count;
	uint32_t i_blocks;
	uint32_t i_flags;
	uint32_t i_osd1;
	uint32_t i_block[15];
	uint32_t i_generation;
	uint32_t i_file_acl;
	uint32_t i_dir_acl;
	uint32_t i_faddr;
	uint8_t i_osd2[12];
};

struct dm_ext2_superblock_t {
	uint32_t s_inodes_count;		// 0
	uint32_t s_blocks_count;		// 4
	uint32_t s_r_blocks_count;		// 8
	uint32_t s_free_blocks_count;		// 12
	uint32_t s_free_inodes_count;		// 16
	uint32_t s_first_data_block;		// 20
	uint32_t s_log_block_size;		// 24
	uint32_t s_log_frag_size;		// 28
	uint32_t s_blocks_per_group;		// 32
	uint32_t s_frags_per_group;		// 36
	uint32_t s_inodes_per_group;		// 40
	uint32_t s_mtime;			// 44
	uint32_t s_wtime;			// 48
	uint16_t s_mnt_count;			// 52
	uint16_t s_max_mnt_count;		// 54
	uint16_t s_magic;			// 56
	uint16_t s_state;			// 58
	uint16_t s_errors;			// 60
	uint16_t s_minor_rev_level;		// 62
	uint32_t s_lastcheck;			// 64
	uint32_t s_checkinterval;		// 68
	uint32_t s_creator_os;			// 72
	uint32_t s_rev_level;			// 76
	uint16_t s_dev_resuid;			// 80
	uint16_t s_dev_resgid;			// 82
	uint32_t s_first_ino;			// 84
	uint16_t s_inode_size;			// 88
	uint16_t s_block_group_nr;		// 90
	uint32_t s_feature_compat;		// 92
	uint32_t s_feature_incompat;		// 96
	uint32_t s_feature_ro_compat;		// 100
	uint8_t s_uuid[16];			// 104 
};

struct dm_ext2_data_t {
	struct dm_dev_t* parent;
	uint64_t block_offset;
	uint64_t block_count;
	struct dm_ext2_superblock_t* sb;
};

static struct dm_dev_t* mod_create_device(struct dm_dev_t*,uint32_t num,uint64_t,uint64_t);

static uuid_t dm_ext2_get_uuid(struct dm_dev_t*);
static int32_t dm_ext2_init(struct dm_dev_t*);
static struct dm_file_t* dm_ext2_open(struct dm_dev_t*,struct dm_path_t*);
static int32_t dm_ext2_read(struct dm_dev_t*,uint64_t,uint32_t,uint32_t,void*);
static uint32_t dm_ext2_blocksize(struct dm_dev_t*);

static int32_t dm_ext2_dir_find(struct dm_dev_t*, struct dm_path_t*, struct dm_ext2_inode_t*,uint32_t* );
static int32_t dm_ext2_inode_block_index(struct dm_dev_t*,struct dm_ext2_inode_t*,uint32_t, uint32_t*);
static int32_t dm_ext2_find_file(struct dm_dev_t*,struct dm_path_t*,struct dm_ext2_inode_t*);
static int32_t dm_ext2_load_inode(struct dm_dev_t*,uint32_t,struct dm_ext2_inode_t*);

static uint64_t dm_ext2_file_read(struct dm_file_t*,void*,uint64_t);
static int32_t dm_ext2_file_close(struct dm_file_t*);
static int32_t dm_ext2_file_llseek(struct dm_file_t*,uint64_t,uint64_t*,int);

static struct dm_dev_method_t dm_ext2_method = {
        .init = dm_ext2_init,
	.open = dm_ext2_open,
	.read = dm_ext2_read,
	.blocksize = dm_ext2_blocksize,
	.get_uuid = dm_ext2_get_uuid
};

static struct dm_mod_method_t mod_method = {
	.create_device = mod_create_device
};

static struct dm_file_method_t file_method = {
	.read = dm_ext2_file_read,
	.close = dm_ext2_file_close,
	.llseek = dm_ext2_file_llseek
};

DECLARE_DM_MODULE("ext2",0x83,mod_method);

/*
 * Initialisation d'une partition
 */
static struct dm_dev_t* mod_create_device(struct dm_dev_t* parent_device, uint32_t num,uint64_t offset, uint64_t size) {
	struct dm_dev_t* p;
	struct dm_ext2_data_t* d;

	p = (struct dm_dev_t*) malloc(sizeof(struct dm_dev_t));
	d = (struct dm_ext2_data_t*) malloc(sizeof(struct dm_ext2_data_t));

	if (p == 0 || d == 0) {
		if (p != 0) free(p);
		if (d != 0) free(d);

		errno = ENOMEM;
		return 0;
	}
	
	memset(p, 0, sizeof(struct dm_dev_t));
	memset(d, 0, sizeof(struct dm_ext2_data_t));

	d->parent = parent_device;
	d->block_offset = offset;
	d->block_count = size;

	p->data = d;
	p->meth = &dm_ext2_method;

	return p;
}

/*
 * Device initialisation
 */
static int32_t dm_ext2_init(struct dm_dev_t* dev) {
	struct dm_ext2_data_t* d = dev->data;

	uint32_t cr;
	
	char uuid[40];

	d->sb = (struct dm_ext2_superblock_t*) malloc(sizeof(struct dm_ext2_superblock_t));
	if (d->sb == 0) {
		errno = ENOMEM;
		return 0;
	}

	cr = dm_read(d->parent,d->block_offset,1024,sizeof(struct dm_ext2_superblock_t),d->sb);
	if (cr<0) {
		free(d->sb);
		d->sb = 0;
		return 0;
	}

	if ( d->sb->s_magic != EXT2_SUPER_MAGIC) {
		errno = EIO;
		free(d->sb);
		d->sb = 0;
		return 0;	
	}

	uuid_unparse(d->sb->s_uuid,uuid);

	DEBUG("EXT2 UUID                  : %s\n",uuid);

	dev->flags |= DM_FLAG_FS;
	
	return 0;
}

/*
 * Get block size
 */
uint32_t dm_ext2_blocksize(struct dm_dev_t* dev) {
	struct dm_ext2_data_t* d = dev->data;
	return 1024 << d->sb->s_log_block_size;
}

/*
 * Get device uuid
 */
static uuid_t dm_ext2_get_uuid(struct dm_dev_t* dev) {
	struct dm_ext2_data_t* d = dev->data;

	if (d->sb) {
		return (uuid_t)&d->sb->s_uuid;
	}

	return 0;
}

/*
 * ext read block
 */
int32_t dm_ext2_read(struct dm_dev_t* dev,uint64_t block,uint32_t offset,uint32_t size,void* buffer) {
	struct dm_ext2_data_t* d = dev->data;

	size_t pbs;

	if (d == 0 && d->sb == 0 && d->parent == 0) {
		errno = EINVAL;
		return -1;
	}

	pbs = dm_blocksize(d->parent);

	pbs = (1024 << d->sb->s_log_block_size) / pbs;

	return dm_read(d->parent,block * pbs + d->block_offset,offset,size,buffer);
}

/*
 * Open file
 */
static struct dm_file_t* dm_ext2_open(struct dm_dev_t* dev,struct dm_path_t* path) {
	int32_t cr;
	struct dm_ext2_inode_t inode;
	struct dm_file_t* file;

	dm_path_next(path);
	DEBUG("EXT2 opening %.*s\n",dm_path_size(path),dm_path_name(path));
	cr = dm_ext2_find_file(dev, path, &inode);
	if (cr < 0) {
		return 0;
	}

	file = (struct dm_file_t*) malloc(sizeof(struct dm_file_t));	
	if (file == 0) {
		errno = ENOMEM;
		return 0;
	}

	file->data = (void*)malloc(sizeof(struct dm_ext2_inode_t));
	if (file->data == 0) {
		errno = ENOMEM;
		free(file);
		return 0;
	}

	memmove(file->data,&inode,sizeof(struct dm_ext2_inode_t));
	file->dev = dev;
	file->meth = &file_method;
	file->offset = 0;

	return file;
}

static int32_t dm_ext2_inode_block_index(struct dm_dev_t* dev,struct dm_ext2_inode_t* i,uint32_t bi, uint32_t* bn) {
	uint32_t bs = dm_blocksize(dev) / sizeof(uint32_t);
	uint32_t bt;
	uint32_t bl;
	int32_t cr;

	struct dm_ext2_data_t* d = dev->data;

	if (bi < 12) {
		*bn = i->i_block[bi];
		return 0;
	}
	bi-= 12;
	bl = bs;

	if (bi < bl) {
		// Look simple indirect
		cr = dm_read(dev,i->i_block[12],bi*sizeof(uint32_t),sizeof(uint32_t),bn);
		if (cr < 0) {
			return -1;
		}
		return 0;
	}

	bi -= bl;
	bl *= bs;

	if (bi < bl) {
		// Look double indirect
		cr = dm_read(dev,i->i_block[13], (bi / bs) * sizeof(uint32_t),sizeof(uint32_t),&bt);
		if (cr<0) {
			return -1;
		} 

		cr = dm_read(dev,bt, (bi % bs) * sizeof(uint32_t), sizeof(uint32_t),bn);
		if (cr<0) {
			return -1;
		}

		return 0;
	}

	bi -= bl;
	bl *= bs;
	
	// Look triple indirect
	cr = dm_read(dev, i->i_block[14], (bi / bs /bs) * sizeof(uint32_t),sizeof(uint32_t), &bt);
	if (cr<0) {
		return -1;
	}	

	cr = dm_read(dev, bt, (bi % (bs*bs)) * sizeof (uint32_t), sizeof(uint32_t),&bt);
	if (cr<0) {
		return -1;
	}

	cr = dm_read(dev, bt, (bi % bs ) * sizeof(uint32_t), sizeof(uint32_t), bn);
	if (cr < 0) {
		return -1;
	}

	return 0;
}

static int32_t dm_ext2_dir_find(struct dm_dev_t* dev, struct dm_path_t* path, struct dm_ext2_inode_t* inode,uint32_t* nindex) {
	struct dm_ext2_data_t* data = dev->data;
	struct dm_ext2_dent_t* dent;

	const char *s;
	size_t n;

	uint32_t bindex;
	uint32_t bcount;
	uint32_t bsize;
	uint32_t baddr;
	uint32_t j;

	uint8_t* buffer;

	int32_t cr;

	s = dm_path_name(path);
	n = dm_path_size(path);
	
	DEBUG("Looking for %.*s\n",n,s);

	bsize = dm_blocksize(dev);
	bcount = inode->i_blocks / (2 << data->sb->s_log_block_size);

	buffer = (uint8_t*) malloc(bsize);
	if (buffer == 0) {
		errno = ENOMEM;
		return -1;
	}

	for (bindex = 0; bindex < bcount ; bindex ++) {
		cr = dm_ext2_inode_block_index(dev,inode,bindex,&baddr);
		if (cr < 0) {
			return -1;
		}

		cr = dm_read(dev,(uint64_t)baddr,0,bsize,buffer);
		if (cr < 0) break;

		for (j=0 ; j < bsize; ) {
			dent = (struct dm_ext2_dent_t*)(buffer + j);
			DEBUG("%08x %02x %3d | %.*s\n", dent->inode,dent->type, dent->length, dent->namelength,buffer+j+8);
			if (dent->namelength == n && strncmp((char*)buffer+j+8,s,n) == 0) {
				*nindex=dent->inode;
				cr = dm_ext2_load_inode(dev,dent->inode,inode);
				if (cr < 0) goto failed;

				return 0;	
			}
			j+= dent->length;
		}
	}
	errno = ENOENT;
failed:
	free(buffer);
	return -1;
}

static int32_t dm_ext2_find_file(struct dm_dev_t* dev,struct dm_path_t* path,struct dm_ext2_inode_t* inode) {
	uint32_t iindex = 2;
	uint32_t nindex = 2;

	int32_t sc = 0;
	int32_t cr;

	// Start with root inode
	cr = dm_ext2_load_inode(dev,iindex,inode);
	if (cr < 0) {
		return -1;
	}

	while (dm_path_hasnext(path)) {
		DEBUG("Looking for [%.*s] in inode %d\n",dm_path_size(path),dm_path_name(path), iindex);

		cr = dm_ext2_dir_find(dev,path,inode,&nindex);
		if (cr < 0) {
			return -1;
		}

		if ((inode->i_mode & EXT2_S_FLAGS) == EXT2_S_IFLNK) {
			if (++sc == 8) {
				errno = ELOOP;
				return -1;
			}

			if (inode->i_size < 60) {
				DEBUG("symlink = <%.*s>\n",inode->i_size,(const char*)&inode->i_block[0]);
				dm_path_next(path);
				path = dm_path_prepend(path,(const char*)&inode->i_block[0]);

				// Reloading previous inode
				cr = dm_ext2_load_inode(dev,iindex,inode);
				if (cr<0) {
					return -1;
				}
				continue;
			} else {
				error("symlink size = %d %08x\n",inode->i_size,inode->i_block[0]);
				error("NOT IMPLEMENTED - WAITING FOR FILE OPEN\n");
				return -1;
			}
		} else if ((inode->i_mode & EXT2_S_FLAGS) == EXT2_S_IFREG) {
			dm_path_next(path);
			if (dm_path_hasnext()) {
				errno = EISDIR;
				return -1;
			}
		} else if ((inode->i_mode & EXT2_S_FLAGS) == EXT2_S_IFDIR) {
			dm_path_next(path);
		} else {
			errno = ENOENT;
			return -1;
		}
		iindex = nindex;
	}
	return 0;
}

/*
 * Inode loading
 */
static int32_t dm_ext2_load_inode(struct dm_dev_t* dev,uint32_t i,struct dm_ext2_inode_t* pi) {
	struct dm_ext2_bdt_t bd;
	struct dm_ext2_data_t* d = dev->data;

	int32_t cr;
	uint32_t bg;
	uint32_t ii;
	uint32_t bs;

	if (i > d->sb->s_inodes_count) {
		errno = EIO;
		return -1;
	}

	bs = 1024 / ( 1024 << d->sb->s_log_block_size) + 1;
	bg = (i-1) / d->sb->s_inodes_per_group;
	ii = (i-1) % d->sb->s_inodes_per_group;

	cr = dm_read(dev,bs,bg * sizeof(bd),sizeof(bd),&bd);
	if (cr < 0) {
		return -1;
	}

	cr = dm_read(dev,bd.bg_inode_table,ii*sizeof(*pi),sizeof(*pi),pi);
	if (cr < 0)  {
		return -1;
	}

	DEBUG("EXT2 inode %08x loaded\n",i);
	return 0;
}

/*
 * file llseek
 */
static int32_t dm_ext2_file_llseek(struct dm_file_t* file,uint64_t offset,uint64_t* result,int whence) {
	struct dm_ext2_inode_t* inode = (struct dm_ext2_inode_t*) file->data;
	uint64_t limit;

	limit = inode->i_size + ((uint64_t)inode->i_dir_acl << 32);

	if (whence == SEEK_SET) {
		file->offset = offset;
	} else if (whence == SEEK_CUR) {
		file->offset += offset;
	} else if (whence == SEEK_END) {
		file->offset = limit + offset;
	} else {
		errno = EINVAL;
		return -1;
	}	

	if (result != 0) {
		*result = file->offset;
	}

	return 0;
}

/*
 * File close
 */
static int32_t dm_ext2_file_close(struct dm_file_t* file) {
	free(file->data);
	free(file);
	return 0;
}

/*
 * File read
 */
static uint64_t dm_ext2_file_read(struct dm_file_t* file,void* buffer,uint64_t size) {
	uint64_t res = 0;
	
	uint64_t limit;
	uint32_t bsize;
	uint64_t bmask;
	uint32_t bindex;
	uint32_t boffset;
	uint32_t offset;
	uint32_t count;

	int32_t cr;

	struct dm_ext2_inode_t* inode = (struct dm_ext2_inode_t*) file->data;
	struct dm_ext2_superblock_t* super = ((struct dm_ext2_data_t*) file->dev->data)->sb;

	bsize = dm_blocksize(file->dev);

	limit = inode->i_size + ((uint64_t)inode->i_dir_acl << 32);

	// Read 0 bytes if after the end of file
	if (file->offset > limit) return 0;

	if (file->offset + size > limit ) {
		size = limit - file->offset;
	}

	offset = (uint32_t)((file->offset) & ((uint64_t) -1) >> (54 - super->s_log_block_size));
	bindex = (uint32_t)(file->offset >> (10 + super->s_log_block_size));

	while (size > 0) {
		cr = dm_ext2_inode_block_index(file->dev,inode,bindex,&boffset);
		if (cr < 0) {
			return res; 
		}

		count = (size < bsize - offset ? size : bsize - offset);

		cr = dm_read(file->dev,boffset,offset,count,buffer);
		if (cr < 0) {
			return res;
		}

		buffer += count;
		file->offset += count;
		size -= count;
		res += count;
		bindex++;
		offset = 0;
	}

	return res;	
}
