TARGET(elf64-x86-64)
ENTRY(_kstart)
SECTIONS
{
	. = 0xffffffffc0000000;

	.text : {

		_text = .;
		*(.head)
		*(.text)
		*(.rodata*)
	}

	.eh_frame : {
		*(.eh_frame)
		. = ALIGN(4096);
	}

	.data : { 
		_data = .;

		*(.data) 
		. = ALIGN(4096);
	}

	.bss : {
		_bss = .;

		*(.bss)
		*(COMMON)

		. = ALIGN(4096);

	}

	_end = .;

	/DISCARD/ : {
		*(.comment)
	}
}



