
JPGS=$(wildcard starwars/*.jpg)

all: starwars.vga

# to generate the .jpg images
# ffmpeg -i recurse/StarWars-OpeningScene-1080pHD.avi -r 30 -s vga -f image2 '%7d.jpg'

starwars.vga: $(JPGS:.jpg=.vga)
	cat starwars/*.vga > $@

%.vga: %.ans ans2vga
	./ans2vga $< $@

%.ans: %.jpg
	img2txt $< > $@

ans2vga: ans2vga.c
	gcc -o $@ $<

clean:
	rm *.vga



