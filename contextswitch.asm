[bits 32]

global asm_switch_to
global asm_fork
extern clone_page_directory

kernel_ss dw 0x10
kernel_esp dd 0x7ffff

; ebx is context_t
%macro save_context 0
		; starting in user stack
    pushf    ; eflags

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

		; store stack pointer and cr3 in task struct
		mov ax, ss
    mov [ebx], ax
    mov [ebx+4], esp

		mov eax, cr3
    mov [ebx+8], eax
%endmacro


%macro restore_context 0
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
		popf
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

		; switch to the kernel stack
		; we'll switch back to the application stack when we load the new context
		mov ax, [kernel_ss]
		mov ss, ax
		mov esp, [kernel_esp]

		; we know out of save context, ebx is the context we want to save to
		push ebx
		call clone_page_directory
		pop ebx

		restore_context
		mov eax, 0
		ret

asm_switch_to:
		save_context

; ebx is the context_t *
asm_restore_context:
		restore_context
		mov eax, 1
		ret
