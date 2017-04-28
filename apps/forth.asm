global main
extern c_print_num

RETURN_STACK:
		dd 0x0
		dd 0x0
		dd 0x0
		dd 0x0

main:
		; top level concepts in registers at all times
		; return stackpointer - ebp
		; data stack stackpointer - esp
		; top of the stack value (TOS) - ebx
		; forth program counter - esi
		mov ebp, RETURN_STACK
		mov esi, init
		jmp NEXT

ENTER:
		mov [ebp], esi
		add ebp, 4
		add ecx, 4
		mov esi, ecx
		jmp NEXT

EXIT:
		dd ASM_EXIT

ASM_EXIT:
		sub ebp, 4
		mov esi, [ebp]
		jmp NEXT

NEXT:
		mov ecx, [esi]
		add esi, 4
		jmp [ecx]

BYE:
		dd ASM_BYE

ASM_BYE:
		cli
		hlt

DOLITERAL:
		dd ASM_DOLITERAL

ASM_DOLITERAL:
		push ebx
		mov ebx, [esi]
		add esi, 4
		jmp NEXT

PRINT:
		dd ASM_PRINT

ASM_PRINT:
		push ebx
		call c_print_num
		pop ebx
		jmp NEXT

DUP:
		dd ASM_DUP

ASM_DUP:
		push ebx
		jmp NEXT

STAR:
		dd ASM_STAR

ASM_STAR:
		pop eax
		mul ebx
		mov ebx, eax
		jmp NEXT

SQUARED:
		dd ENTER
		dd DUP, STAR, EXIT

init:
		dd DOLITERAL
		dd 42
		dd SQUARED
		dd PRINT
		dd BYE
