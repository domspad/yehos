all: boot.img

%.bin: %.asm
	nasm -f bin -l $@.lst -o $@ $<

%.img: bootloader.bin
	cat $< bunchofzeroes.bin > $@

clean:
	rm bootloader.bin boot.img
