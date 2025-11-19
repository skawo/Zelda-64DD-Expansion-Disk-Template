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

SRCS    = ddTool.c

ADDRESS = 0x80000400
__EZLJ_ERROR_IPL_BIN_ADDR = 

CFLAGS = -G 0 -Os -I../../include/libc -I../../include -fno-toplevel-reorder --std=gnu99 -mtune=vr4300 -mabi=32 -mips3 -mno-check-zero-division -fno-builtin -mno-explicit-relocs -mno-memcpy -fno-reorder-blocks -ffast-math -fno-unsafe-math-optimizations -mno-check-zero-division -fno-optimize-sibling-calls

LDFLAGS = -T entry.ld --emit-relocs

default: ddTool.o

ddTool.o: ddTool.c
	@$(CC) -c ddTool.c -B$(CC1_DIR) $(CFLAGS)

clean:
	rm -f *.bin *.elf *.o
