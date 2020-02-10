/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion int8 -> ascii
 */

#include "loader/utl/conv.h"

const char* conv_int8_h(int8_t val,const char* buffer,size_t len) {
	char* p;
	p = conv_hexa(val,buffer,len,0);
	return p;
}
