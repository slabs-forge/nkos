/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion uint32 -> ascii
 */

#include "loader/utl/conv.h"

const char* conv_uint32_h(uint32_t val,const char* buffer,size_t len) {
	char* p;
	p = conv_hexa(val,buffer,len,0);
	return p;
}
