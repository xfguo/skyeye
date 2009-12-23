/*
 * hello.lds
 * ld script for helloforSkyeye
 * 
 * author: SU Hang
 * Date:   2004-08-28
 */

OUTPUT_ARCH(arm)
ENTRY(begin)
SECTIONS
{
	. = 0x1000000;
	.text : 
	{
		*(.text) 
		*(.rodata)
	}


	. = ALIGN(8192);

	.data : {*(.data)}

	.bss : {*(.bss)}


        /* Stabs debugging sections.    */
        .stab 0 : { *(.stab) }
        .stabstr 0 : { *(.stabstr) }
        .stab.excl 0 : { *(.stab.excl) }
        .stab.exclstr 0 : { *(.stab.exclstr) }
        .stab.index 0 : { *(.stab.index) }
        .stab.indexstr 0 : { *(.stab.indexstr) }
        .comment 0 : { *(.comment) }
        .debug_abbrev 0 : { *(.debug_abbrev) }
        .debug_info 0 : { *(.debug_info) }
        .debug_line 0 : { *(.debug_line) }
        .debug_pubnames 0 : { *(.debug_pubnames) }
        .debug_aranges 0 : { *(.debug_aranges) }

}
