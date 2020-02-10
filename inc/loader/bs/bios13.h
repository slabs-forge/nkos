/*
 * NK Loader - bios services
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * BIOS 13h services
 *
 */

#ifndef __LOADER_BS_BIOS13_H__
#define __LOADER_BS_BIOS13_H__

#include "loader/types.h"

struct bs13_drive_parms_t {
	uint16_t size;
	uint16_t flags;
	uint32_t cylinders;
	uint32_t heads;
	uint32_t sectors;	
	uint64_t lba;
	uint16_t bps;
};

int32_t bs13_get_drive_parms(uint8_t,struct bs13_drive_parms_t*);

int32_t bs13_read_sect(uint8_t,uint64_t,uint16_t,void*);

#endif

