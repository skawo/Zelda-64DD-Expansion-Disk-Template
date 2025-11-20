ifeq ($(OS),Windows_NT)
EXE=win
else
EXE=linux
endif

CC      = ../tool/gcc/$(EXE)/mips64-gcc
LD      = ../tool/gcc/$(EXE)/mips64-ld
OBJCOPY = ../tool/gcc/$(EXE)/mips64-objcopy
OBJDUMP = ../tool/gcc/$(EXE)/mips64-objdump
CC1_DIR = ../tool/gcc/$(EXE)

SRCS    = diskCode.c filesystem.c 
CFLAGS = -G 0 -Os -I../include/libc -I../include -fno-toplevel-reorder --std=gnu99 -mtune=vr4300 -mabi=32 -mips3 -mno-check-zero-division -fno-builtin -mno-explicit-relocs -mno-memcpy -fno-reorder-blocks -ffast-math -fno-unsafe-math-optimizations -mno-check-zero-division -fno-optimize-sibling-calls
LDFLAGS = -T disk.ld --emit-relocs

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

BOOT_OFFS_VALUE = 0x738C0
TIMESTAMP = "{0x00,0x$(YEAR1),0x$(YEAR2),0x$(MONTH),0x$(DAY),0x$(HOUR),0x$(MIN),0x$(SEC)}"
OUTFILENAME = "EZLE.ndd"

default: USA

USA: common

JPN: DISKREGION = DISKREGION_JPN
JPN: OUTFILENAME = "EZLJ.ndd"
JPN: common

USA-DEV: DISKRELEASE = DISKRELEASE_DEV
USA-DEV: OUTFILENAME = "EZLE-DEV.ndd"
USA-DEV: common

JPN-DEV: DISKREGION = DISKREGION_JPN
JPN-DEV: DISKRELEASE = DISKRELEASE_DEV
JPN-DEV: OUTFILENAME = "EZLJ-DEV.ndd"
JPN-DEV: common

USA-D64: DISKFORMAT = DISKFORMAT_D64
USA-D64: OUTFILENAME = "EZLE.d64"
USA-D64: d64

JPN-D64: DISKFORMAT = DISKFORMAT_D64
JPN-D64: DISKREGION = DISKREGION_JPN
JPN-D64: OUTFILENAME = "EZLJ.d64"
JPN-D64: d64

common: clean-diskSystem ddTool/ddTool.o filesystem/filesystem.o ddTool/ddToolCode.o diskCode/diskCode.o diskBoot/diskBoot.o diskSystem/diskSystem.o
	@rm -f ../*.disk
	@echo "Creating $(OUTFILENAME)..."
	@$(LD) -o disk.elf ddTool/ddTool.o filesystem/filesystem.o ddTool/ddToolCode.o diskCode/diskCode.o diskBoot/diskBoot.o diskSystem/diskSystem.o --defsym=BOOT_OFFS_VALUE=0x738C0 $(LDFLAGS)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary disk.elf ../$(OUTFILENAME)
	@dd if=/dev/zero bs=1 count=0 seek=$$((0x3DEC800)) of=../$(OUTFILENAME) 2>/dev/null

d64: clean-diskSystem ddTool/ddTool.o filesystem/filesystem.o ddTool/ddToolCode.o diskCode/diskCode.o diskBoot/diskBoot.o diskSystem/diskSystem.o
	@rm -f ../*.disk
	@echo "Creating $(OUTFILENAME)..."
	@$(LD) -o disk.elf ddTool/ddTool.o filesystem/filesystem.o ddTool/ddToolCode.o diskCode/diskCode.o diskBoot/diskBoot.o diskSystem/diskSystem.o --defsym=BOOT_OFFS_VALUE=0x200 $(LDFLAGS)
	@$(OBJCOPY) -R .MIPS.abiflags -O binary disk.elf ../$(OUTFILENAME)

ddTool/ddTool.o: ddTool/ddTool.c
	@echo "Compiling tools..."
	@$(CC) -DPREFIX=boot_ -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

ddTool/ddToolCode.o: ddTool/ddTool.c
	@$(CC) -DPREFIX=ram_ -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

filesystem/filesystem.o: filesystem/filesystem.s
	@echo "Compiling filesystem..."
	@$(CC) -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

diskCode/diskCode.o: diskCode/diskCode.c diskCode/functionReplacements.c diskCode/versionedCode.c diskCode/ddScenes.c
	@echo "Compiling diskCode..."
	@$(CC) -DPREFIX=ram_ -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

diskBoot/diskBoot.o: diskBoot/diskBoot.c
	@echo "Compiling diskBoot..."
	@$(CC) -DPREFIX=boot_ -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

diskSystem/diskSystem.o: diskSystem/diskSystem.c
	@echo "Compiling diskSystem..."
	@$(CC) -D$(DISKREGION) -D$(DISKRELEASE) -D$(DISKFORMAT) -DTIMESTAMP=$(TIMESTAMP) \
	       -D__IPL_ENTRY=0x$(__IPL_ENTRY_POINT) \
	       -c $< -o $@ -B$(CC1_DIR) $(CFLAGS)

clean-diskSystem:
	@rm -f diskSystem/*.o

clean:
	@rm -f ../*.disk ../*.ndd ../*.d64
	@rm -f ddTool/*.o filesystem/*.o diskCode/*.o diskBoot/*.o diskSystem/*.o