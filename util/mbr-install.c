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

#include <stdio.h>
#include <stdlib.h>

#define MBR_MAXSIZE	440
#define BLOCK_SIZE	512
#define MAGIC		0xAA55

#define EC_OK		0
#define EC_BADARGS 	1
#define EC_IFILE	2
#define EC_OFILE	3

/*
 * print the usage
 */
void usage() {
	printf("usage: mbr-install [binary mbr] [disk image]\n");
}

/*
 * main
 */
int main(int argc, char* argv[]) {
	unsigned short magic = MAGIC;

	char mbr[MBR_MAXSIZE];
	
	int cr;
	ssize_t c;

	int ifd;
	int ofd;

	struct stat istat;
	struct stat ostat;

	if (argc != 3) {
		usage();
		exit(EC_BADARGS);
	}

	ifd = open(argv[1], O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr , "Can't open MBR image file - errno=%d\n", errno);
		exit(EC_IFILE);
	}

	cr = fstat(ifd, &istat);
	if (cr < 0) {
		fprintf(stderr, "Can't stat MBR image file - errno = %d\n", errno);
		close(ifd);
		exit(EC_IFILE);
	}

	if (istat.st_size > MBR_MAXSIZE) {
		fprintf(stderr, "MBR image file too long\n");
		close(ifd);
		exit(EC_IFILE);
	}

	memset(&mbr,0,MBR_MAXSIZE);

	c = read(ifd,&mbr,MBR_MAXSIZE);
	if (c < 0) {
		fprintf(stderr, "Can't read MBR image file - errno=%d\n", errno);
		exit(EC_IFILE);
	}
	close(ifd);

	ofd = open(argv[2], O_RDWR);
	if (ofd < 0) {
		fprintf(stderr , "Can't open  disk image file - errno=%d\n", errno);
		exit(EC_OFILE);
	}

	cr = fstat(ofd, &ostat);
	if (cr < 0) {
		fprintf(stderr, "Can't stat disk image file - errno = %d\n", errno);
		close(ofd);
		exit(EC_OFILE);
	}

	if (ostat.st_size < BLOCK_SIZE) {
		fprintf(stderr, "disk image file too short\n");
		close(ofd);
		exit(EC_OFILE);
	}

	c = write(ofd,&mbr,MBR_MAXSIZE);
	if (c < 0) {
		fprintf(stderr, "Can't write disk image file - errno=%d\n", errno);
		close(ofd);
		exit(EC_OFILE);
	}
	
	if (lseek(ofd,BLOCK_SIZE-2,SEEK_SET) != (BLOCK_SIZE-2)) {
		fprintf(stderr, "Can't write magic number to disk image file - errno=%d\n", errno);
		close(ofd);
		exit(EC_OFILE);
	}

	c = write(ofd,&magic,sizeof(magic));
	if (c < 0) {
		fprintf(stderr, "Can't write disk image file - errno=%d\n", errno);
		close(ofd);
		exit(EC_OFILE);
	}

	close(ofd);
	exit(EC_OK);
}

