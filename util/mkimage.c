/*
 * NK utils
 *
 * Build a kernel image from start.bin and kernel.bin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define EC_OK		0
#define EC_BADARGS	1
#define EC_IFILE	2
#define EC_OFILE	3

#define PAGE_SIZE 4096

/*
 * print usage
 */
void usage() {
	printf("usage: mkimage [start.bin] [kernel.bin] [kernel]\n");
}

int process(int sfd,int kfd,int ofd) {
	int cr;
	ssize_t rd;
	struct stat istat;
	struct stat kstat;

	off_t off;

	size_t spos;
	size_t slimit;
	size_t kpos;
	size_t klimit;

	char buffer[PAGE_SIZE];

	cr = fstat(sfd,&istat);
	if (cr < 0) {
		fprintf(stderr,"Can't stat kernel starter - errno=%d\n", errno);
		return EC_IFILE;
	}

	cr = fstat(kfd,&kstat);
	if (cr < 0) {
		fprintf(stderr,"Can't stat kernel main - errno=%d\n", errno);
		return EC_IFILE;
	}

	slimit = (istat.st_size + 4095) & ~4095;

	printf("Kernel - Starter - Size = %d bytes\n",istat.st_size);
	printf("Kernel - Main    - Size = %d bytes\n",kstat.st_size);

	slimit = (istat.st_size + 4095) & ~4095;
	klimit = (kstat.st_size + 4095) & ~4095;

	for (spos = 0; spos < slimit; spos += PAGE_SIZE) {
		memset(buffer,0,sizeof(buffer));

		printf("Reading at %ld\n",spos);
		rd = read(sfd,buffer,PAGE_SIZE);
		if (rd == - 1) {
			printf("Can't read kernel stater - errno=%d\n", errno);
			return EC_IFILE;
		}

		rd = write(ofd,buffer,PAGE_SIZE);
		if (rd != PAGE_SIZE) {
			printf("Can't write kernel image - errno=%d\n", errno);
			return EC_OFILE;
		}
	}

	for (kpos = 0; kpos < klimit ; kpos += PAGE_SIZE) {
		memset(buffer, 0, sizeof(buffer));

		rd = read(kfd, buffer, PAGE_SIZE);
		if (rd == -1) {
			printf("Can't read kernel main - errno=%d\n", errno);
			return EC_IFILE;
		}

		rd = write(ofd, buffer, rd);
		if (rd == -1) {
			printf("Can't write kernel main - errno=%d\n", errno);
			return EC_OFILE;
		}
		
	}

	off = lseek(ofd,22,SEEK_SET);
	if (off != 22) {
		printf("Can't write kernel size - errno=%d\n", errno);
		return EC_OFILE;
	}

	rd = write(ofd, &slimit,sizeof(slimit));
	if (rd != sizeof(slimit)) {
		printf("Can't write kernel size - errno=%d\n", errno);
		return EC_OFILE;
	}

	rd = write(ofd, &klimit,sizeof(klimit));
	if (rd != sizeof(klimit)) {
		printf("Can't write kernel size - errno=%d\n", errno);
		return EC_OFILE;
	}
	return 0;
	return 0;
}

/*
 * main
 */
int main(int argc, char* argv[]) {
	int sfd;
	int kfd;
	int ofd;

	if (argc != 4) {
		usage();
		exit(EC_BADARGS);
	}

	sfd = open(argv[1], O_RDONLY);
        if (sfd < 0) {
                fprintf(stderr , "Can't open kernel starter - errno=%d\n", errno);
                exit(EC_IFILE);
        }

	kfd = open(argv[2], O_RDONLY);
        if (kfd < 0) {
                fprintf(stderr , "Can't open kernel - errno=%d\n", errno);
                exit(EC_IFILE);
	}

	ofd = open(argv[3], O_RDWR | O_CREAT,0760);
        if (ofd < 0) {
                fprintf(stderr , "Can't open  kernel image file - errno=%d\n", errno);
                exit(EC_OFILE);
        }

	return process(sfd,kfd,ofd);
}
