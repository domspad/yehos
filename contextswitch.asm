[bits 32]

; passed context_t *
asm_restore_context:
    mov ebx, [esp+4] ; context_t *target_ctx
    mov eax, [ebx]   ; ss
    mov ecx, [ebx+4] ; esp
    mov edx, [ebx+8] ; cr3

    mov cr3, edx     ; go to new address space
    mov ss, ax
    mov esp, ecx     ; replace stack

    pop ds
    pop es
    pop fs
    pop gs

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    iret

; args: context_t *current_ctx, *target_ctx
asm_switch_to:
    pop eax  ; return addr
    pop ebx  ; context_t *

    pushf    ; eflags
    push cs  ; cs:
    push eax ; repush return addr for eip

    push eax ; clobbered
    push ebx ; clobbered
    push ecx
    push edx
    push esi
    push edi
    push ebp

    push gs
    push fs
    push es
    push ds

    mov [ebx], ss
    mov [ebx+4], esp
    mov [ebx+8], cr3

