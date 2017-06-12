

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
