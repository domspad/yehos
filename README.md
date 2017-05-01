

# Yehos
## an operating system for the people

We are building an operating system from scratch under the guidance of @saulpw and his work on [frotzos](https://github.com/saulpw/frotzos). This is our story...

## Building

Linux users shouldn't have any problem building the system. However, Macs will have some difficulty. For this reason, it is suggested that Macs first build a [cross-compiler](https://en.wikipedia.org/wiki/Cross_compiler). This will allow them to generate 32-bit ELF binary files.

Either:

run `sudo yehos/tools/build/cross_compiler.sh` (the cross compiler will be installed to `/opt/cross/`), or execute the commands in the file manually. 
