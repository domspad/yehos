[bits 32]

global asm_switch_to
global asm_fork
extern dup_context

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
		; clobber ecx now to move ss around - we already pushed it
		mov cx, ss
    mov [eax], cx
    mov [eax+4], esp

		mov ecx, cr3
    mov [eax+8], ecx
%endmacro


%macro restore_context 0
    mov eax, [edx]   ; ss
    mov ecx, [edx+8] ; cr3

		cli
    mov cr3, ecx     ; go to new address space
    mov ss, ax
    mov esp, [edx+4]     ; replace stack
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
		mov eax, [esp + 4] ; original context
		mov edx, [esp + 8] ; new context
		save_context

		; switch to the kernel stack
		; we'll switch back to the application stack when we load the new context
		mov cx, [kernel_ss]
		mov ss, cx
		mov esp, [kernel_esp]

		; we know out of save context, ebx is the context we want to save to
		push edx ; new context - we'll dupe the stale context into here
		push eax ; original context - we just wrote to it
		call dup_context
		pop edx  ; pop the original context into edx - we'll restore it now
		pop eax 

		; restore context expects to find a target context in edx
		restore_context
		mov eax, 0
		ret

asm_switch_to:
		mov eax, [esp + 4] ; stale context
		mov edx, [esp + 8] ; target context
		save_context

		; restore context expects to find a target context in edx
		restore_context
		mov eax, 1
		ret
