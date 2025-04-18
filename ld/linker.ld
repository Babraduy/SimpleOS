ENTRY(_start_kernel_entry)

SECTIONS
{
	.text 0x1000 : {
		*(.text*)
	}

	.rodata : {
		*(.rodata*)
	}

	.data : {
		*(.data*)
	}

	.bss : {
		*(.bss*)
		*(COMMON)
	}
}
