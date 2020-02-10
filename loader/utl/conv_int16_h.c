/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion int32 -> ascii
 */

#include "loader/utl/conv.h"

const char* conv_int16_h(int16_t val,const char* buffer,size_t len) {
	char* p;
	p = conv_decimal(val,buffer,len,0);
	return p;
}
