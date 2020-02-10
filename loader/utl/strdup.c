/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 11/09/2011
 *
 * strdup
 */

#include "loader/utl/string.h"
#include "loader/utl/stdlib.h"

char* strdup(const char* s) {
	char * p = 0;
	size_t l = strlen(s)+1;

	p = (char*) malloc(l);
	if (p == 0) {
		return p;
	}

	strcpy(p,s);
	return p;
}
