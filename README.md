

# Yehos
## an operating system for the people

We are building an operating system from scratch under the guidance of @saulpw and his work on [frotzos](https://github.com/saulpw/frotzos). This is our story...

## Requirements

 - The [qemu](www.qemu.org/) emulator (depending on installation, you may have to also need `qemu-arch-extra` to support the i386 architecture. 

 - `nasm` and `ndisasm` (come together)

 - `mkisofs` command line tool

 - The [Star Wars vga file](https://github.com/zormit/yehos/pull/4#issuecomment-307857771) for the movie.

## Building

Linux users shouldn't have any problem building the system. However, Macs will have some difficulty. For this reason, it is suggested that Macs first build a [cross-compiler](https://en.wikipedia.org/wiki/Cross_compiler). This will allow them to generate 32-bit ELF binary files.

so Macs... it you can either:

run `sudo yehos/tools/build/cross_compiler.sh` (the cross compiler will be installed to `/opt/cross/`)

or

execute the commands in the file manually. 

## Memory Layout

### Physical Memory

? - 0x6000: bootstack
0x7c00 - 0x7dff: boot sector
0x8000 kernel

0x80000 starting page directory
0x81000 first page table
0x100000 page pool

### Virtual Memory
0 - 0x100000: identity mapped to physical memory
0x100000 - ?: disk iso
0x1000000: entry point to applications
0xffbfe000 - 0xffbfefff: reserved for copying to physical memory
0xffbff000 - 0xffbfffff: application stack
0xffc00000 - end: page table

