/*
 * NK utils (for linux)
 * Copy the boot loader into MBR sector (image of disk)
 */

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>

#define PARTITION_LOCATION 	446
#define MBR_MAXSIZE		440
#define BLOCK_SIZE		512

#define EC_OK		0
#define EC_BADARGS 	1
#define EC_IFILE	2
#define EC_OFILE	3
#define EC_COPY		4

struct partition_t {
	unsigned char flag;
	unsigned char sh;
	unsigned char ss;
	unsigned char sc;
	unsigned char type;
	unsigned char eh;
	unsigned char es;
	unsigned char ec;
	unsigned int start;
	unsigned int length;
};

/*
 * print the usage
 */
void usage() {
	printf("usage: stage1-install [binary stage1] [disk image] [kernel]\n");
}

/*
 * copy sector
 */
int copy_sector(int ifd,int ofd,int isect,int osect) {
	ssize_t c;

	char sect[BLOCK_SIZE];

	off_t ipos = isect * BLOCK_SIZE;
	off_t opos = osect * BLOCK_SIZE;


	if (lseek(ifd,ipos,SEEK_SET) != ipos) {
		return -1;
	}

	if (lseek(ofd,opos,SEEK_SET) != opos) {
		return -1;
	}

	memset(sect, 0, BLOCK_SIZE);

	c = read (ifd, &sect, BLOCK_SIZE);
	if (c < 0) {
		return -1;
	}

	c = write(ofd,&sect, BLOCK_SIZE);
	if (c < 0) {
		return -1;
	}
	
	return 0;
}

/*
 * process
 */
int process(int ifd, int ofd) {
	int i;
	ssize_t c;
	struct partition_t parts[4];

	unsigned int np = 0;
	unsigned int ms = UINT_MAX;
	unsigned int sc;

	int cr;

	struct stat istat;
	struct stat ostat;

	cr = fstat(ifd, &istat);
	if (cr < 0) {
		fprintf(stderr, "Can't stat state1 image file - errno = %d\n", errno);
		return EC_IFILE;
	}

	sc = (istat.st_size - 1) / 512 + 1;

	cr = fstat(ofd, &ostat);
	if (cr < 0) {
		fprintf(stderr, "Can't stat disk image file - errno = %d\n", errno);
		return -1;
	}

	if (ostat.st_size < BLOCK_SIZE) {
		fprintf(stderr, "disk image file too short\n");
		return EC_OFILE;
	}

	if (lseek(ofd,PARTITION_LOCATION,SEEK_SET) < 0) {
		fprintf(stderr, "can't read partition table\n");
		return EC_OFILE;
	}	

	c = read(ofd, &parts, sizeof(parts));
	if (c != sizeof(parts)) {
		fprintf(stderr, "can't read partition table\n");
		return EC_OFILE;
	}	

	for (i=0; i<4; i++) {
		if (parts[i].type == 0x00) {
			continue;
		}
		np++;

		if (parts[i].start < ms) {
			ms = parts[i].start;
		}
	}

	if (np == 0) {
		fprintf(stderr, "no valid partition defined on disk image\n");
		return EC_OFILE;
	}

	
	ms = ms - 1;

	if (sc > ms) {
		fprintf(stderr, "Not enough space for stage1\n");
		fprintf(stderr, "stage1 requirement : %d sectors\n",sc);
		fprintf(stderr, "disk available sectors : %d\n",ms);
		return EC_OFILE;
	}

	for (i = 0 ; i < sc ; i++) {
		if (copy_sector(ifd,ofd,i, i+1) < 0) {
			fprintf(stderr,"Can't copy sector to disk image\n");
			return EC_COPY;
		} 
	}

	return EC_OK;
}

/*
 * update bootparm
 */
int update_bootparm(int ofd, const char* kernel) {
	off_t off;
	size_t l;
	ssize_t w;

	off = lseek(ofd, 0x0008 + BLOCK_SIZE, SEEK_SET);
	if (off != 0x0008 + BLOCK_SIZE) {
		fprintf(stderr, "Can't write boot parameters\n");
		return EC_COPY;
	}

	l = strlen(kernel);
	w = write(ofd, kernel, l);
	if (w != l) {
		fprintf(stderr, "Can't write boot parameters\n");
		return EC_COPY;
	}	

	return 0;
}

/*
 * main
 */
int main(int argc, char* argv[]) {
	int cr;

	int ifd;
	int ofd;

	if (argc != 4) {
		usage();
		exit(EC_BADARGS);
	}

	if (strlen(argv[3]) > 127) {
		fprintf(stderr, "Kernel image filename too long\n");
		exit(EC_BADARGS);
	}

	ifd = open(argv[1], O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr , "Can't open stage1 image file - errno=%d\n", errno);
		exit(EC_IFILE);
	}

	ofd = open(argv[2], O_RDWR);
	if (ofd < 0) {
		fprintf(stderr , "Can't open  disk image file - errno=%d\n", errno);
		close(ifd);
		exit(EC_OFILE);
	}

	cr = process(ifd,ofd);
	
	update_bootparm(ofd, argv[3]);

	close(ifd);
	close(ofd);

	exit(cr);
}

