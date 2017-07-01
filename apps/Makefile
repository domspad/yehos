
CFLAGS+=-ffreestanding
CFLAGS+=-m32
CFLAGS+=-nostdlib
CFLAGS+=-nostdinc
CFLAGS+=-nostartfiles
CFLAGS+=-nodefaultlibs
CFLAGS+=-fno-strict-aliasing
CFLAGS+=-isystem ..
CFLAGS+=-O1
CFLAGS+=-ggdb

all: hello.bin txtplayr.bin forth.bin

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
CROSS_CC = gcc
CC = gcc
LD = ld
OBJDUMP = objdump
OBJCOPY = objcopy
endif
ifeq ($(UNAME_S),Darwin)
CROSS_CC = i386-elf32-gcc
CC = gcc-6.3.0
LD = i386-elf32-ld
OBJDUMP = i386-elf32-objdump
OBJCOPY = i386-elf32-objcopy
endif

.SECONDARY: hello.o crt0.o hello.lst txtplayr.lst

forth.elf: forth.o forthprint.o
txtplayr.elf: txtplayer.o video.o

hello.elf: hello.o

%.elf: ../libyehos.o | crt0.o
	$(LD) -m elf_i386 -T linker.ld -o $@ $^

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

%.lst: %.elf
	$(OBJDUMP) -d $< > $@

%.bin: %.elf %.lst
	$(OBJCOPY) -O binary $< $@

.c.o:
	$(CROSS_CC) -c $(CFLAGS) -o $@ $<

.s.o:
	nasm -f elf32 -o $@ $<

clean:
	rm -f *.o *.elf *.bin
