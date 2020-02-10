OUTPUT_ARCH(i386)
TARGET(elf32-i386)
SECTIONS
{
	. = 0x8000;

	_top = .;	
	.text : { 
		.obj/start.o(.head)
		. = 0x0200;
		.obj/start.o(.text)
		*(.text)
	 }
	.ctors : { 
		PROVIDE(_CTOR_LIST = .);
		KEEP (*(SORT(.ctors.*)))
		KEEP(*(.ctors))
	}
	_CTOR_SIZE = SIZEOF(.ctors) / 4 ;
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	PROVIDE(_sect = (. - _top -1 ) / 512 + 1);
	.bss : { *(.bss) }
}



