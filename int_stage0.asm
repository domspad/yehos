[BITS 32]

global exc_stage0_start, exc_stage0_fixup, exc_stage0_end
global excerr_stage0_start, excerr_stage0_fixup, excerr_stage0_end
global asm_halt

extern exception_handler

exc_handler_ptr dd exception_handler

asm_halt:
    hlt
    jmp asm_halt

excerr_stage0_start:
    xchg eax, [esp]            ; eax = error code
    pushad                     ; save all registers
    push eax                   ; arg2 = errcode
excerr_stage0_fixup:    
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handler_ptr]      ; exception_handler(exc#, errcode, ...)
excerr_stage0_end equ $

exc_stage0_start:
    push eax                   ; for consistency with above
    pushad                     ; save all registers
    push eax                   ; arg2 = fake errcode for consistency
exc_stage0_fixup:              ; "mov eax" is only 1 byte, literal follows
    mov eax, 0
    push eax                   ; arg1 = exception #
    push exception_end         ; unified return address
    jmp [exc_handler_ptr]      ; exception_handler(exc#, errcode, ...)
exc_stage0_end equ $

global exception_end
exception_end:
    pop eax
    pop eax
    popad
    pop eax
    iret



