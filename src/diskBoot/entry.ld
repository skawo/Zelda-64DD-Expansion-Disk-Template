RAM_ORIGIN  = 0x80000400;  
RAM_LENGTH  = 0x4D08;   /*  Same as BLK_SIZE_ZONE0 (the boot code can only take the one sector) */

OUTPUT_ARCH(mips)
OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")

MEMORY {
    RDRAM (rw) : ORIGIN = RAM_ORIGIN, LENGTH = RAM_LENGTH
}

SECTIONS {
    . = ALIGN(32);
    .text : ALIGN(32) 
    {
        __entry = .;
        *(.text*)
        *(.init)
        *(.fini)
    } > RDRAM

    . = ALIGN(32);
    .rodata : ALIGN(32) 
    {
        *(.rodata*)
        *(.eh_frame*)
    } > RDRAM

    . = ALIGN(32);
    .data : ALIGN(32) 
    {
        *(.data*)
        *(.sdata*)
    } > RDRAM

    . = ALIGN(32);
    .bss : ALIGN(32) 
    {
        *(COMMON)
        *(.bss*)
        *(.sbss*)
    } > RDRAM

    . = ALIGN(32);
    __end = .; 

    __IPL_Entry = __entry;
}
