ROM_LENGTH = 0x3F000000;    

OUTPUT_ARCH(mips)
OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")

MEMORY {
    ROM (rw) : ORIGIN = 0, LENGTH = ROM_LENGTH
}

SECTIONS {
    .diskHeader : ALIGN(1) 
    {
        KEEP(*(.diskHeader))
    } > ROM   
}
