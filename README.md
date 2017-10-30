# Yehos
## An operating system for the people

We are building an operating system from scratch under the guidance of @saulpw and his work on [frotzos](https://github.com/saulpw/frotzos). This is our story...

## Components

* Bootloader
* Keyboard IO
* Video output
* Lazy loading of applications from ISO disk
* Virtual memory and demand paging
* Multiple concurrent processes

Yehos supports two applications:

* A video player playing a text-encoded version of the film Star Wars (1977).
* A REPL for the Forth programming language, implemented in x86 assembly.

## Requirements

 - The [qemu](www.qemu.org/) emulator (depending on installation, you may have to also need `qemu-arch-extra` to support the i386 architecture. 

 - `nasm` and `ndisasm` (come together)

 - `mkisofs` command line tool

 - The [Star Wars vga file](https://github.com/zormit/yehos/pull/4#issuecomment-307857771) for the movie.

## Building

Linux users can simply run `make`.

It is suggested that Macs first build a [cross-compiler](https://en.wikipedia.org/wiki/Cross_compiler). This will allow them to generate 32-bit ELF binary files.

Mac users can either:

run `sudo yehos/tools/build/cross_compiler.sh` (the cross compiler will be installed to `/opt/cross/`)

or

execute the commands in the file manually. 

## Running

`make run` will run Yehos normally.

To debug Yehos with GDB, run
```
qemu-system-i386 -s -S -cdrom yehos-patched.iso &
gdb
```

The contents of the `.gdbinit` file point gdb to target qemu (localhost:1234).

It is possible to get a console for interacting with qemu by following the instructions [here](https://en.wikibooks.org/wiki/QEMU/Monitor). Running `info tlb` in the console is especially helpful for debugging virtual memory mappings.

## Memory Layout

### Physical Memory

* ? - 0x6000: bootstack
* 0x7c00 - 0x7dff: boot sector
* 0x8000 kernel
* 0x80000 starting page directory
* 0x81000 first page table
* 0x100000 page pool

### Virtual Memory

* 0 - 0x100000: identity mapped to physical memory
* 0x100000 - ?: disk iso
* 0x1000000: entry point to applications
* 0xffbfe000 - 0xffbfefff: reserved for copying to physical memory
* 0xffbff000 - 0xffbfffff: application stack
* 0xffc00000 - end: page table

