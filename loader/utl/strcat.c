/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * strcat
 */

#include "loader/utl/string.h"

char* strcat(char* dest, const char* src) {
	size_t l = strlen(dest);
	strcpy(dest + l,src);
	return dest;
}
