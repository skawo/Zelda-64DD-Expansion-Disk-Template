ifeq ($(OS),Windows_NT)
EXE=win
else
EXE=linux
endif

CC      = ../../tool/gcc/$(EXE)/mips64-gcc
LD      = ../../tool/gcc/$(EXE)/mips64-ld
OBJCOPY = ../../tool/gcc/$(EXE)/mips64-objcopy
OBJDUMP = ../../tool/gcc/$(EXE)/mips64-objdump
CC1_DIR = ../../tool/gcc/$(EXE)

SRCS    = diskBoot.c

ADDRESS = 0x80000400
__EZLJ_ERROR_IPL_ADDR = 

CFLAGS = -G 0 -Os -I../../include/libc -I../../include -fno-toplevel-reorder --std=gnu99 -mtune=vr4300 -mabi=32 -mips3 -mno-check-zero-division -fno-builtin -mno-explicit-relocs -mno-memcpy -fno-reorder-blocks -ffast-math -fno-unsafe-math-optimizations -mno-check-zero-division -fno-optimize-sibling-calls

LDFLAGS = -T entry.ld --emit-relocs

default: diskBoot.bin

diskBoot.bin: diskBoot.o 
	@$(LD) -o diskBoot.elf diskBoot.o ../ddTool/ddTool.o --just-symbols ../filesystem/filesystem.o $(LDFLAGS)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary diskBoot.elf diskBoot.bin

diskBoot.o: diskBoot.c
	$(eval __EZLJ_ERROR_IPL_ADDR := $(shell $(OBJDUMP) -t ../diskCode/diskCode.elf | grep '__ErrorIPL_Start' | awk '{print $$1}'))
	@$(CC) -D__EZLJ_ERROR_IPL_ADDR=0x$(__EZLJ_ERROR_IPL_ADDR) -c diskBoot.c -B$(CC1_DIR) $(CFLAGS)

clean:
	rm -f *.bin *.elf *.o
