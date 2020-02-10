/*
 * NK Loader - Kernel functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * Kernel error handling
 */

#include "loader/types.h"
#include "loader/loader.h"
#include "loader/errno.h"
#include "loader/stdarg.h"

#include "loader/utl/stdio.h"

uint32_t errno = 0;

void error(const char* format,...) {
        va_list ap;

	printf("\033[31m");
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
	printf("\033[0m");

}

void panic(const char* format,...) {
        va_list ap;

	printf("\033[31mPANIC: ");
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
	printf("\033[0m");
	halt();
}

void debug(const char* format,...) {
        va_list ap;

	printf("\033[32;4m");
        va_start(ap, format);
        vfprintf(stdout, format, ap);
        va_end(ap);
	printf("\033[0m");

}
