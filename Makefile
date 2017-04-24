
KERNEL_CFLAGS += -O1
KERNEL_CFLAGS += -ggdb
KERNEL_CFLAGS += -m32
KERNEL_CFLAGS += -ffreestanding
KERNEL_CFLAGS += -nostdlib
KERNEL_CFLAGS += -nostdinc
KERNEL_CFLAGS += -nostartfiles
KERNEL_CFLAGS += -nodefaultlibs
KERNEL_CFLAGS += -fno-strict-aliasing
KERNEL_CFLAGS += -std=gnu99
KERNEL_CFLAGS += -isystem .

KERNEL_OBJS= kb.o vgatext.o exceptions.o syscalls.o int_stage0.o memlib.o interrupts.o video.o ata.o iso9660.o kprint.o debug.o virtualmem.o


all: yehos-patched.iso

bootloader.bin: bootloader.asm
	nasm -f bin -l bootloader.lst -o bootloader.bin bootloader.asm

kernel.bin: kmain.o kernel.ld $(KERNEL_OBJS)
	ld -m elf_i386 -T kernel.ld -o kernel.elf $(KERNEL_OBJS)
	objdump -d --disassembler-options=intel kernel.elf > kernel.lst
	objcopy -O binary kernel.elf $@

yehos-patched.iso: yehos.iso isopatcher
	cp $< $@
	./isopatcher $@ kernel.bin

yehos.iso: kernel.bin bootloader.bin apps
	mkisofs \
		-r \
		-iso-level 1 \
		-no-pad \
		-b bootloader.bin \
		-c boot.cat \
		-no-emul-boot \
		-boot-load-seg=0x7c0 \
		-boot-load-size=1 \
		-boot-info-table \
		-input-charset=iso8859-1 \
		-o $@ bootloader.bin kernel.bin vga/starwars.vga apps/hello.bin

isopatcher: isopatcher.c iso9660.c iso9660.h
	gcc -ggdb -o $@ $^

# for kernel
.c.o:
	gcc -c $(KERNEL_CFLAGS) -o $@ $<

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

apps:
	make -C apps

run: yehos-patched.iso
	qemu-system-i386 -cdrom $<

clean:
	rm -f bootloader.bin kernel.bin kernel.elf yehos.iso *.lst *.map *.o isopatcher

