/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion int32 -> ascii
 */

#include "loader/utl/conv.h"

const char* conv_int16_d(int16_t val,const char* buffer,size_t len) {
	size_t rlen;
	char* p;
	unsigned int v;

	if (val < 0) {
		v = (unsigned int) -val;
	} else {
		v = (unsigned int) val;
	}

	p = conv_decimal(v,buffer,len,&rlen);
	if (p == 0) {
		return 0;
	}

	if (val >= 0) {
		return p;
	} else {
		if (rlen == 0) {
			return 0;
		} else {
			p--;
			*p = '-';
			return p;
		}
	}
}

