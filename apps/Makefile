
CFLAGS+=-ffreestanding
CFLAGS+=-m32
CFLAGS+=-nostdlib
CFLAGS+=-nostdinc
CFLAGS+=-nostartfiles
CFLAGS+=-nodefaultlibs
CFLAGS+=-fno-strict-aliasing

all: hello.bin

hello.elf: hello.o

%.elf:
	ld -m elf_i386 -T linker.ld -o $@ $^

%.lst: %.elf
	objdump -d $< > $@

%.bin: %.elf %.lst
	objcopy -O binary $< $@

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

.s.o:
	nasm -f elf32 -o $@ $<
