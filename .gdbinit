set history save on
set history size 256
target remote localhost:1234
file kernel.elf
break kmain
layout src
continue
