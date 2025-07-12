XT_PRG_PREFIX = mipsel-linux-gnu-
CC = $(XT_PRG_PREFIX)gcc
LD = $(XT_PRG_PREFIX)ld

UMPS3_DATA_DIR = ./umps

ifneq ($(wildcard /usr/bin/umps3),)
    UMPS3_DIR_PREFIX = /usr
else
    UMPS3_DIR_PREFIX = /usr/local
endif

UMPS3_DATA_DIR = $(UMPS3_DIR_PREFIX)/share/umps3
UMPS3_INCLUDE_DIR = $(UMPS3_DIR_PREFIX)/include/umps3

LIB := lib
LIB2 := phase1
vpath %.h lib phase1

CFLAGS_LANG = -ffreestanding -DTARGET_UMPS=1
CFLAGS_MIPS = -mips1 -mabi=32 -mno-gpopt -G 0 -mno-abicalls -fno-pic -mfp32
CFLAGS = $(CFLAGS_LANG) $(CFLAGS_MIPS) -I$(LIB) -I$(LIB2) -I$(UMPS3_INCLUDE_DIR) -Wall -O0

LDFLAGS = -G 0 -nostdlib -T $(UMPS3_DATA_DIR)/umpscore.ldscript -m elf32ltsmip

VPATH = $(UMPS3_DATA_DIR)

vpath %.c phase1 phase1/p1test


.PHONY : all clean

all : kernel.core.umps

kernel.core.umps : kernel
	umps3-elf2umps -k $<

kernel : p1test.o pcb.o ash.o ns.o crtso.o libumps.o
	$(LD) -o $@ $^ $(LDFLAGS)

clean :
	-rm -f kernel kernel.*.umps *.o umps/*.o *.umps

%.o : %.S
	$(CC) $(CFLAGS) -c -o $@ $<