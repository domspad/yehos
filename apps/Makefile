
CFLAGS+=-ffreestanding
CFLAGS+=-m32
CFLAGS+=-nostdlib
CFLAGS+=-nostdinc
CFLAGS+=-nostartfiles
CFLAGS+=-nodefaultlibs
CFLAGS+=-fno-strict-aliasing

all: hello.bin

.SECONDARY: hello.o crt0.o hello.lst

hello.elf: hello.o

%.elf: | crt0.o
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
