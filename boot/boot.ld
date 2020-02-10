
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
TARGET("elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
	. = 0x7C00;
	.text : { *(.text) }
	.data : { *(.data) }
	
}
ASSERT(SIZEOF(.text)+SIZEOF(.data) < 440,"too long")



