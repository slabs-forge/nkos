/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 26/09/2011
 *
 * serial port handling
 */

#include "kernel/types.h"
#include "kernel/serial.h"
#include "kernel/io.h"

void serial_init() {
	uint16_t port = 0x3f8;

	/*
	 * Set Baud Rate to 115200
	 */
	outb(port + 3 , 0x80);
	outb(port + 0 , 0x01);
	outb(port + 1 , 0x00);
	outb(port + 3 , 0x00);

	/*
	 * Setting  8N1
	 */
	outb(port + 3, 0x03);

	/*
	 * Setting up FIFO
	 */
	outb(port + 2, 0xc7);
}

void serial_putchar(int c) {
	uint16_t port = 0x3f8;
	uint8_t lsr;
	for (;;) {
		lsr = inb(port + 5);
		if ((lsr & 0x20) == 0x20) break;
	}
	outb(port + 0, c);
}

