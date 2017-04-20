
CFLAGS += -O1
CFLAGS += -ggdb
CFLAGS += -m32
CFLAGS += -ffreestanding
CFLAGS += -nostdlib
CFLAGS += -nostdinc
CFLAGS += -nostartfiles
CFLAGS += -nodefaultlibs
CFLAGS += -fno-strict-aliasing
CFLAGS += -std=gnu99
CFLAGS += -isystem .

KERNEL_OBJS= kb.o vgatext.o exceptions.o int_stage0.o memlib.o interrupts.o video.o ata.o iso9660.o kprint.o debug.o virtualmem.o


all: yehos-patched.iso

bootloader.bin: bootloader.asm
	nasm -f bin -l bootloader.lst -o bootloader.bin bootloader.asm

kernel.bin: kmain.o kernel.ld $(KERNEL_OBJS)
	i386-elf32-ld  -m elf_i386 -T kernel.ld -o kernel.elf $(KERNEL_OBJS)
	i386-elf32-objdump -d --disassembler-options=intel kernel.elf > kernel.lst
	i386-elf32-objcopy -O binary kernel.elf $@

yehos-patched.iso: yehos.iso isopatcher
	cp $< $@
	./isopatcher $@ kernel.bin

yehos.iso: kernel.bin bootloader.bin
	mkisofs \
		-r \
		-iso-level 1 \
		-no-pad \
		-boot-load-size=1 \
		-boot-load-seg=0x7c0 \
		-boot-info-table \
		-b bootloader.bin \
		-c boot.cat \
		-no-emul-boot \
		-input-charset=iso8859-1 \
		-o $@ bootloader.bin kernel.bin vga/starwars.vga

isopatcher: isopatcher.c iso9660.c iso9660.h
	gcc-6.3.0 -ggdb -o $@ $^

.c.o:
	i386-elf32-gcc-6.3.0 -c $(CFLAGS) -o $@ $<

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

run: yehos-patched.iso
	qemu-system-i386 -cdrom $<

clean:
	rm -f bootloader.bin kernel.bin kernel.elf yehos.iso *.lst *.map *.o isopatcher
