
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

KERNEL_OBJS= kb.o vgatext.o exceptions.o int_stage0.o memlib.o interrupts.o

all: yehos.img

bootloader.bin: bootloader.asm
	nasm -f bin -l bootloader.lst -o bootloader.bin bootloader.asm

kernel.bin: kmain.o kernel.ld $(KERNEL_OBJS)
	ld -m elf_i386 -T kernel.ld -o kernel.elf $(KERNEL_OBJS)
	objdump -d --disassembler-options=intel kernel.elf > kernel.lst
	objcopy -O binary kernel.elf $@

yehos.img: kernel.bin bootloader.bin
	cat bootloader.bin kernel.bin > $@
	truncate --size=4KB $@
	cat vga/0000200.vga vga/0010000.vga >> $@
	truncate --size=128KB $@

.c.o:
	gcc -c $(CFLAGS) -o $@ $<

%.o: %.asm
	nasm $(ASMFLAGS) -f elf -o $@ $<

run: yehos.img
	qemu-system-i386 -drive format=raw,file=$<

clean:
	rm -f bootloader.bin kernel.bin kernel.elf yehos.img *.lst *.map *.o
