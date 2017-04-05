
CFLAGS += -ggdb
CFLAGS += -ffreestanding
CFLAGS += -m32
CFLAGS += -nostdlib
CFLAGS += -nostdinc
CFLAGS += -nostartfiles
CFLAGS += -nodefaultlibs
CFLAGS += -fno-strict-aliasing


all: yehos.img

bootloader.bin: bootloader.asm
	nasm -f bin -l bootloader.lst -o bootloader.bin bootloader.asm

kernel.bin: kmain.o kernel.ld
	ld -m elf_i386 -T kernel.ld -o kernel.elf kmain.o
	objdump -d kernel.elf > kernel.lst
	objcopy -O binary kernel.elf $@

yehos.img: kernel.bin bootloader.bin
	cat bootloader.bin kernel.bin > $@
	truncate --size=1MB $@

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

clean:
	rm -f bootloader.bin kernel.bin kernel.elf yehos.img *.lst *.map *.o
