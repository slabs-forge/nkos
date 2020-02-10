/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion ascii -> uuid
 */

#include "loader/errno.h"

#include "loader/utl/uuid.h"
#include "loader/utl/string.h"

int32_t uuid_parse(const char* in,uuid_t uu) {
	uint32_t i;
	uint32_t j = 0;
	uint8_t v;

	// Reset the uuid
	memset(uu,0,16);

	for (i=0;i<36;i++) {
		if (i == 8 || i == 13 || i == 18 || i == 23) {
			if (in[i] == '-') continue;
			else {
				errno = EINVAL;
				return -1;
			}
		}

		if (in[i] >= '0' && in [i] <='9') {
			v = in[i] - '0';			
		} else if (in[i] >= 'a' && in[i] <= 'f') {
			v = in[i] - 'a' + 10;
		} else {
			errno = EINVAL;
			return -1;
		}

		uu [j>>1] |= ( (j & 0x1) == 0x1 ? v : v << 4);
		j++;
	}
	return 0;
}

