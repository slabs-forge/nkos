/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 17/09/2011
 *
 * Link script de l'amorce
 */
OUTPUT_FORMAT("elf32-i386","elf32-i386","elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	PROVIDE (_top = .);
	.text : {
		*(.head)
		*(.text)
	} 

        .rodata : { *(.rodata) }
        .data : {
		 *(.data) 
		 *(.bss)
		 *(COMMON)
	}


	. = ALIGN(4);

	PROVIDE(_size = . - _top );
}


