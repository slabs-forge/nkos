/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion uuid -> ascii
 */
#include "loader/utl/uuid.h"

void uuid_unparse(uuid_t val,char* out) {
	uint32_t i;
	char* v = out;

	for (i = 0 ; i < 16; i++) {
		*v = (val[i] >> 4);

		if (*v >= 10) *v += 'a'-10;
		else *v += '0';
		
		v++;	

		*v = (val[i] & 0x0F);

		if (*v >= 10) *v += 'a'-10;
		else *v += '0';
		
		v++;

		if (i == 3 || i == 5 || i == 7 || i == 9) {
			*v++ = '-';
		}
	}
	*v = 0;
}
