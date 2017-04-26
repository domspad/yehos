
extern main

global _start
global _exit
_start:
    mov ebp, 0
    push ebp
    mov ebp, esp

    call main

    push eax   ; return value from main
    call _exit

; already on the stack as a parameter is the exit code
_exit:
    mov eax, 0
    int 30h   ; syscall 0 == exit

