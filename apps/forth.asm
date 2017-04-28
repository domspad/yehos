global main
extern c_print_num
main:
		; top level concepts in registers at all times
		; return stackpointer - edx
		; data stack stackpointer - esp
		; top of the stack value (TOS) - ebx
		; forth program counter - esi
		mov esi, init
		jmp NEXT

FORTYTWO:
		; push a 42 onto stack
		push ebx
		mov ebx, 42
		jmp NEXT

BYE:
		cli
		hlt

NEXT:
		mov ecx, [esi]
		add esi, 4
		jmp ecx

PRINT:
		push ebx
		call c_print_num
		pop ebx
		jmp NEXT

DUP:
		push ebx
		jmp NEXT

STAR:
		pop eax
		mul ebx
		mov ebx, eax
		jmp NEXT

init:
		dd FORTYTWO
		dd DUP
		dd STAR
		dd PRINT
		dd BYE
