[bits 32]

global asm_save_context, asm_load_context

asm_swap_context: ; save to fromctx, load from toctx
    pushf
    push esi
    push edi

    mov edi, [esp+4] ; TODO: fromctx
    mov esi, [esp+8] ; TODO: find right address

    mov ax, ds
    stosd
    mov ax, es
    stosd
    mov ax, fs
    stosd
    mov ax, gs
    stosd

    mov ax, ss
    stosd

    mov eax, ebx
    stosd
    mov eax, ecx
    stosd
    mov eax, edx
    stosd

    pop eax ; old edi
    stosd
    pop eax ; old esi
    stosd

    mov eax, ebp
    stosd

    mov eax, cr3
    stosd

; now start loading old one

    lodsd


asm_load_context:
    mov esp, eax
    add esp, 4    ; remove return address from esp given back to C

    pop eax
    mov cr3, eax

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    mov eax, 0
    pop gs
    pop fs
    pop es
    pop ss
    pop ds

    iret ; pop EFLAGS/CS/EIP

