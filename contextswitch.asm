[bits 32]

global asm_swap_context

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

    pop eax     ; eflags
    stosd

    mov ax, cs
    stosd

    pop eax     ; eip (return addr)
    stosd

    ; now start loading old one
    
    lodsd
    mov ds, ax
    lodsd
    mov es, ax
    lodsd
    mov fs, ax
    lodsd
    mov gs, ax
    lodsd
    mov ss, ax

    lodsd
    mov ebx, eax
    lodsd
    mov ecx, eax
    lodsd
    mov edx, eax
    lodsd
    mov edi, eax
    lodsd
    mov esi, eax
    lodsd
    mov ebp, eax

    lodsd
    mov cr3, eax

    lodsd     ; eflags
    push eax
    lodsd     ; code segment
    push eax
    lodsd     ; eip (return address)
    push eax

    iret

