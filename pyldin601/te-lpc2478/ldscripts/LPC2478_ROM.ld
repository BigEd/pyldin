
MEMORY 
{
	FLASH   : ORIGIN = 0x00000000, LENGTH = 512K
	RAM     : ORIGIN = 0x40000000, LENGTH = 64K
	USBRAM  : ORIGIN = 0x7FD00000, LENGTH = 16K
	ETHRAM  : ORIGIN = 0x7FE00000, LENGTH = 16K
}

__stack_end__ = 0x40000000 + 32K - 4;

SECTIONS 
{
	. = 0;

	.startup :
    {
    . = ALIGN(4);
        KEEP(*(.startup)) /* ISR vector table */
	. = ALIGN(4);
    } >FLASH

	.text :
	{
        *(.text)
        *(.rodata) 
        *(.rodata*)
		*(.glue_7)
		*(.glue_7t)
	} >FLASH

    . = ALIGN(4);

    .ctors :
    {
        __ctors_start__ = .;
        *(.init_array)
        /* *(.ctors) */
        __ctors_end__ = .;

        KEEP(SORT(*)(.init_array))
        /* KEEP(SORT(*)(.ctors)) */
    }

    .dtors :
    {
        __dtors_start__ = .;
        /* *(.dtors) */
        *(.fini_array)
        __dtors_end__ = .;

        KEEP(SORT(*)(.fini_array))
        /* KEEP(SORT(*)(.dtors)) */
    }

    . = ALIGN(4);

	__end_of_text__ = .;

	.data : 
	{
		__data_beg__ = .;
		__data_beg_src__ = __end_of_text__;
		*(.data)
		__data_end__ = .;
	} >RAM AT>FLASH

    .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
    __exidx_start = .;
    .ARM.exidx   : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >FLASH
    __exidx_end = .;

	.bss : 
	{
		__bss_beg__ = .;
		*(.bss)
	} >RAM

	/* Align here to ensure that the .bss section occupies space up to
	_end.  Align after .bss to ensure correct alignment even if the
	.bss section disappears because there are no input sections.  */
	. = ALIGN(32 / 8);
	_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;

	.USBRAM (NOLOAD):
	{
        __usbram_beg__ = .;
        *(.dmaram)
		__usbram_end__ = .;
	} >USBRAM

	.ETHRAM (NOLOAD):
	{
        __ethram_beg__ = .;
        *(.ETHRAM)
		__ethram_end__ = .;
	} >ETHRAM


}
	. = ALIGN(32 / 8);
	_end = .;
	
	PROVIDE (end = .);


