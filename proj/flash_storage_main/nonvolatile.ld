SECTIONS
{
	.nonvolatile : {
		. = ALIGN(64); /* align to erase page size */
		_reserved_nv_start = .;
		KEEP(*(.nonvolatile));
		. += (64 * 1); /* (erase page size) * (N pages to reserve) */
		_reserved_nv_end = .;
	} >FLASH AT>FLASH
}
PROVIDE(_reserved_nv_start = _reserved_nv_start);
PROVIDE(_reserved_nv_end = _reserved_nv_end);
