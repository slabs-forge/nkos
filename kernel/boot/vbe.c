/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 22/09/2011
 *
 * vesa ext 3 driver
 */

#include "kernel/boot/code16gcc.h"

#include "kernel/boot/boot.h"
#include "kernel/boot/vbe.h"
#include "kernel/boot/far.h"
#include "kernel/boot/intcall.h"
#include "kernel/boot/utl/stdio.h"
#include "kernel/boot/utl/string.h"

#define VBE_MAGIC_VESA	0x41534556
#define VBE_MAGIC_VBE2	0x32454256

struct vbe_info_t {
	uint32_t signature;
	uint16_t version;
	uint32_t oemstring;
	uint32_t capabilities;
	uint32_t modes;
	uint16_t memory;
	uint16_t rev;
	uint32_t vendor;
	uint32_t product;
	uint32_t prodrev;

	char junk[476];
} __attribute__((packed));

struct vbe_mode_t {
	uint16_t attributes;
	uint8_t winAAttributes;
	uint8_t winBAttributes;
	uint16_t winGranularity;
	uint16_t winSize;
	uint16_t winASegment;
	uint16_t winBSegment;
	uint32_t winFuncPtr;
	uint16_t bytesPerScanLine;
	uint16_t xResolution;
	uint16_t yResolution;
	uint8_t junk[234];
} __attribute__((packed));

int32_t vbe_init(void) {
	struct regs_t regsi;
	struct regs_t regso;
	struct vbe_info_t info;
	struct vbe_mode_t mode;

	struct bootparm_t* bp = get_bootparm();

	uint16_t* p;

	char tmp[256];

	printf("VBE driver initialization...\r\n");

	memset(&regsi,0,sizeof(regsi));
	memset(&regso,0,sizeof(regso));
	memset(&info,0,sizeof(info));

	regsi.ax = 0x4f00;
	regsi.edi = (uint32_t) &info;
	info.signature = VBE_MAGIC_VBE2;
	
	intcall(0x10,&regsi,&regso);

	if (regso.eax != 0x004f && info.signature != VBE_MAGIC_VESA && info.version < 0x200) {
		printf("VBE driver initialization failed\n");
		goto failure;
	}

	far_strncpy(tmp,(const char*) info.oemstring,sizeof(tmp));
	printf("VBE Oem String   : %s\r\n",tmp);
	printf("VBE capabilities : %08x\r\n",info.capabilities);
	printf("VBE modes        : %08x\r\n",info.modes);
	printf("VBE memory       : %8.4x\r\n",info.memory);

	far_strncpy(tmp,(const char*) info.vendor,sizeof(tmp));
	printf("VBE vendor       : %s\r\n",tmp);
	far_strncpy(tmp,(const char*) info.product,sizeof(tmp));
	printf("VBE product      : %s\r\n",tmp);
	far_strncpy(tmp,(const char*) info.prodrev,sizeof(tmp));
	printf("VBE product rev  : %s\r\n",tmp);

	for (p = (uint16_t*) info.modes;;p++) {
		uint16_t id = far_readw(p);
		if (id == 0xffff) break;

		memset(&regsi,0,sizeof(regsi));
		memset(&regso,0,sizeof(regso));
		memset(&mode,0,sizeof(mode));
		regsi.ax = 0x4f01;
		regsi.cx = id;
		regsi.edi = (uint32_t) &mode;

		intcall(0x10,&regsi,&regso);

		if ((mode.attributes & 0x0d) != 0x0d) continue;
		printf("%04x | %04x | %02x | %02x | %04x | %04x | %5d | %5d | %5d\r\n",
			id,
			mode.attributes,
			mode.winAAttributes,
			mode.winBAttributes,
			mode.winGranularity,
			mode.winSize,
			mode.bytesPerScanLine,
			mode.xResolution,
			mode.yResolution
		);
	}

	printf("VBE driver initialization done !!\r\n");

	return 0;
failure:
	return -1;
}

