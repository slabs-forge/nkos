/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Conversion uint16 -> ascii
 */

#include "loader/utl/conv.h"

const char* conv_uint16_h(uint16_t val,const char* buffer,size_t len) {
	return conv_hexa(val,buffer,len,0);
}
