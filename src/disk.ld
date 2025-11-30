KSEG1 = 0xA0000000;
BOOT_ORIGIN  = 0x80000400; 
LBA_LENGTH  = 0x4D08;
RAM_ORIGIN  = 0x80400000; 
ROM_LENGTH = 0x3DEC800;    
RAM_LENGTH  = 0x400000;      

BOOT_OFFS = BOOT_OFFS_VALUE;

OUTPUT_ARCH(mips)
OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")

MEMORY {
    HEADER (rw) : ORIGIN = 0, LENGTH = BOOT_OFFS
    BOOT (rw) : ORIGIN = BOOT_ORIGIN, LENGTH = LBA_LENGTH

    ROM (rw) : ORIGIN = 0, LENGTH = ROM_LENGTH
    RAM (rw) : ORIGIN = RAM_ORIGIN, LENGTH = RAM_LENGTH
}

OFFSET = 0;

SECTIONS {
    .diskHeader : AT(OFFSET) ALIGN(1) 
    {
        KEEP(*(.diskHeader))
    } > HEADER   

    OFFSET += BOOT_OFFS;

    .boot : AT(OFFSET) ALIGN(1) 
    {
        . = ORIGIN(BOOT);
        __IPL_Entry = .;
        KEEP(*diskBoot.o(.text* .rodata* .data* .sdata* .bss* .sbss*))
        KEEP(*ddTool.o(.text* .rodata* .data* .sdata* .bss* .sbss*))
    } > BOOT

    OFFSET += LBA_LENGTH;

    .diskInfo 0 : AT(OFFSET) ALIGN(1) 
    {
        KEEP(*(.diskInfo))
        . = ALIGN(32);
    } > ROM   

    OFFSET += SIZEOF(.diskInfo);

    .rom : AT(OFFSET) ALIGN(32) 
    {
        __romStart = .;
        KEEP(*filesystem.o(.text* .rodata* .data* .sdata* .bss* .sbss*)) 
        KEEP(*vtables.o(.text* .rodata* .data* .sdata* .bss* .sbss*))
        . = ALIGN(32);
    } > ROM
    __romEnd = .;

    OFFSET += SIZEOF(.rom);

    . = ORIGIN(RAM);
    .text : AT(OFFSET)
    {
        __entry = .;
        *(.text*)
        *(.init)
        *(.fini)
    } > RAM
    
    .rodata ALIGN(32) :
    {
        *(.rodata*)
        *(.eh_frame*)
    } > RAM

    .data ALIGN(32) :
    {
        *(.data*)
        *(.sdata*)
    } > RAM

    .bss ALIGN(32) :
    {
        *(COMMON)
        *(.bss*)
        *(.sbss*)
    } > RAM

    . = ALIGN(32);
    __ramEnd = .;
}

__Disk_Init_K1   = (KSEG1 | Disk_Init);
__Disk_Start     = __romEnd;
__Disk_End       = __Disk_Start + __ramEnd - __entry;
__Disk_VramStart = (KSEG1 | __entry);
__Disk_VramEnd   = (KSEG1 | (__entry + __Disk_End - __Disk_Start));
__ErrorIPL_Start = __romStart;