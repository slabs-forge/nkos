/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * Debugging output
 *
 */

#include "kernel/types.h"
#include "kernel/stdarg.h"
#include "kernel/serial.h"
#include "kernel/utl/printf.h"
#include "kernel/utl/string.h"

int64_t kvdebug(const char* format,va_list ap);

/*
 * kdebug
 */
int64_t kdebug(const char* format,...) {
	int64_t result;
	va_list ap;

	va_start(ap,format);
	result = kvdebug(format,ap);
	va_end(ap);

	return result;
}

int64_t kvdebug(const char* format,va_list ap) {

	auto size_t kvdebug_room(size_t size) {
		return size;
	}

	auto void kvdebug_output(int c) {
		serial_putchar(c);
	}

	struct out_t out = { 
		.room = kvdebug_room,
		.output = kvdebug_output,
	};

	return kvprinto(&out,format,ap);
}

