# Detect platform for toolchain
ifeq ($(OS),Windows_NT)
EXE = win
else
EXE = linux
endif

CC      = ../../tool/gcc/$(EXE)/mips64-gcc
LD      = ../../tool/gcc/$(EXE)/mips64-ld
OBJCOPY = ../../tool/gcc/$(EXE)/mips64-objcopy
OBJDUMP = ../../tool/gcc/$(EXE)/mips64-objdump
CC1_DIR = ../../tool/gcc/$(EXE)

SRCS    = diskSystem.c

ADDRESS = 0x80000400
__IPL_ENTRY =
DISKREGION  = DISKREGION_USA
DISKRELEASE = DISKRELEASE_RETAIL
DISKFORMAT  = DISKFORMAT_NDD
BUILDTIME := $(shell date +'%Y %m %d %H %M %S')

YEAR1 := $(shell echo $(BUILDTIME) | awk '{print substr($$1,1,2)}')
YEAR2 := $(shell echo $(BUILDTIME) | awk '{print substr($$1,3,2)}')
MONTH := $(shell echo $(BUILDTIME) | awk '{print $$2}')
DAY   := $(shell echo $(BUILDTIME) | awk '{print $$3}')
HOUR  := $(shell echo $(BUILDTIME) | awk '{print $$4}')
MIN   := $(shell echo $(BUILDTIME) | awk '{print $$5}')
SEC   := $(shell echo $(BUILDTIME) | awk '{print $$6}')

TIMESTAMP = "{0x00,0x$(YEAR1),0x$(YEAR2),0x$(MONTH),0x$(DAY),0x$(HOUR),0x$(MIN),0x$(SEC)}"

CFLAGS = -G 0 -Os -I../../include/libc -I../../include -fno-toplevel-reorder --std=gnu99 \
         -mtune=vr4300 -mabi=32 -mips3 -mno-check-zero-division -fno-builtin -mno-explicit-relocs \
         -mno-memcpy -fno-reorder-blocks -ffast-math -fno-unsafe-math-optimizations \
         -mno-check-zero-division -fno-optimize-sibling-calls

LDFLAGS = -T entry.ld --emit-relocs

default: USA

USA: common

JPN: DISKREGION = DISKREGION_JPN
JPN: common

USA-DEV: DISKRELEASE = DISKRELEASE_DEV
USA-DEV: common

JPN-DEV: DISKREGION = DISKREGION_JPN
JPN-DEV: DISKRELEASE = DISKRELEASE_DEV
JPN-DEV: common

USA-D64: DISKFORMAT = DISKFORMAT_D64
USA-D64: common

JPN-D64: DISKFORMAT = DISKFORMAT_D64
JPN-D64: DISKREGION = DISKREGION_JPN
JPN-D64: common

common: diskSystem.bin

diskSystem.bin: diskSystem.o
	@$(LD) -o diskSystem.elf diskSystem.o --just-symbols ../diskBoot/diskBoot.o $(LDFLAGS)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary diskSystem.elf diskSystem.bin

diskSystem.o: diskSystem.c
	$(eval __IPL_ENTRY_POINT := $(shell $(OBJDUMP) -t ../diskBoot/diskBoot.elf | grep '__IPL_Entry' | awk '{print $$1}'))
	@$(CC) -D$(DISKREGION) -D$(DISKRELEASE) -D$(DISKFORMAT) -DTIMESTAMP=$(TIMESTAMP) \
	       -D__IPL_ENTRY=0x$(__IPL_ENTRY_POINT) \
	       -c diskSystem.c -B$(CC1_DIR) $(CFLAGS)
clean:
	rm -f *.elf *.o diskSystem.bin
