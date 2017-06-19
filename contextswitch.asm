[bits 32]

kernel_ss dw 0x10
kernel_esp dd 0x

; ebx is context_t
%macro save_context 0
		; starting in user stack
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

		; switch to kernel stack
		cli
		mov ax, ss
    mov [ebx], ax

		mov ax, [kernel_ss]
		mov ss, ax

    mov [ebx+4], esp
		mov esp, [kernel_esp]

		mov eax, cr3
    mov [ebx+8], eax
		sti
%endmacro

; FORK
; save off the context to the stack and task struct
;
;	set up a new page directory in prep for swap 
;   - copy pagedir to newly allocated page w/ cow setting
;   - the new cr3 will be written into the new entry in the task struct
; 
; populate the new entry in the task struct with ss, esp, and the new cr3
;
; execute restore context: 
;   load context from the stack (it's the one we just saved)
asm_fork:
		; context_t is first argument
		mov ebx, [esp + 4]

		save_context

		push ebx ; we know out of save context, ebx is the context we want to save to
		call clone_page_directory
		pop ebx

	 	jmp asm_restore_context

asm_switch_to:
		save_context

; ebx is the context_t *
asm_restore_context:

    mov eax, [ebx]   ; ss
    mov ecx, [ebx+4] ; esp
    mov edx, [ebx+8] ; cr3

		cli
    mov cr3, edx     ; go to new address space
    mov ss, ax
    mov esp, ecx     ; replace stack
		sti

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

