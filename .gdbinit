set history save on
set history size 256
target remote localhost:1234
file apps/forth.elf
break main
