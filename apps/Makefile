
CFLAGS+=-ffreestanding
CFLAGS+=-m32
CFLAGS+=-nostdlib
CFLAGS+=-nostdinc
CFLAGS+=-nostartfiles
CFLAGS+=-nodefaultlibs
CFLAGS+=-fno-strict-aliasing
CFLAGS+=-isystem ..
CFLAGS+=-O2
CFLAGS+=-ggdb

all: hello.bin txtplayr.bin

.SECONDARY: hello.o crt0.o hello.lst txtplayr.lst


txtplayr.elf: txtplayer.o video.o

hello.elf: hello.o

%.elf: ../libyehos.o | crt0.o
	ld -m elf_i386 -T linker.ld -o $@ $^

%.lst: %.elf
	objdump -d $< > $@

%.bin: %.elf %.lst
	objcopy -O binary $< $@

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

.s.o:
	nasm -f elf32 -o $@ $<

clean:
	rm -f *.o *.elf *.bin
