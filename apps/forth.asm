global main
extern c_print_num

INPUT:
		dd "DOLI"
		dd 10
		dd "PRIN"
		dd "BYE "

INPUT_PTR dd INPUT

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
		dd "BYE "
		dd 0
		dd 0

ASM_BYE:
		cli
		hlt

DOLITERAL:
		dd ASM_DOLITERAL
		dd "DOLI"
		dd BYE
		dd 0

ASM_DOLITERAL:
		push ebx
		mov ebx, [esi]
		add esi, 4
		jmp NEXT

PRINT:
		dd ASM_PRINT
		dd "PRIN"
		dd DOLITERAL
		dd 0

ASM_PRINT:
		push ebx ; move TOS to top of system stack
						 ; so that it will be considered a param by c_print num
		call c_print_num
		pop ebx  ; get rid of param from system stack
		pop ebx  ; pop the forth stack, print consumes arg
		jmp NEXT

FIND:
		dd ASM_FIND
		dd "FIND"
		dd PRINT
		dd 0

ZERO dd 0
; ( word - addr )
ASM_FIND:
		mov eax, [DICT]
ASM_FIND_RECURSIVE:
		add eax, 4
		push ebx
		mov ebx, 0
		cmp [eax], ebx
		pop ebx
		je ASM_WORD_NOT_FOUND
		cmp [eax], ebx
		je ASM_FIND_MATCH_FOUND
		; not found match
		add eax, 4 ; EAX now points to the address of the previous word in the dictionary.
		mov eax, [eax]
		jmp ASM_FIND_RECURSIVE
		; match found
ASM_FIND_MATCH_FOUND:
		push ebx
		sub eax, 4
		mov ebx, eax
		jmp NEXT
ASM_WORD_NOT_FOUND:
		push ebx
		mov ebx, 0
		jmp NEXT

BLANK:
		dd ASM_BL
		dd "BLAN"
		dd FIND
		dd 0

ASM_BL:
		push ebx
		mov ebx, 0
		jmp NEXT

FWORD:
		dd ASM_WORD
		dd "WORD"
		dd BLANK
		dd 0

; Eventually this should create words by delimiting
; a char buffer by the delimiter on the stack.
ASM_WORD:
		mov ebx, [INPUT_PTR]
		jmp NEXT

; ( 0 | addr -> num | execution ) 
EXEC_OR_PUSH:
		dd ASM_EXEC_OR_PUSH
		dd "EORP"
		dd FWORD
		dd 0

ASM_EXEC_OR_PUSH:
		cmp ebx, 0
		je ASM_PUSH_NUM
		jmp ASM_EXECUTE
ASM_PUSH_NUM:
		pop ebx
ASM_EXECUTE:
		jmp [ebx]

DICT dd EXEC_OR_PUSH


INTERPRET:
		dd ENTER, BLANK, FWORD, FIND, EXEC_OR_PUSH, EXIT



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

; ( a b - a b a )
OVER:
		dd ASM_OVER

ASM_OVER:
		mov eax, [esp]
		push ebx
		mov ebx, eax
		jmp NEXT

; ( a b - b a )
SWAP:
		dd ASM_SWAP

ASM_SWAP:
		pop eax
		push ebx
		mov ebx, eax
		jmp NEXT

; ( a b c - b c a )
ROT:
		dd ASM_ROT

ASM_ROT:
		mov eax, ebx
		pop ebx
		push eax

		mov eax, ebx
		add esp, 4
		pop ebx ; a is correct position
		push eax ; c is in correct position
		sub esp, 4
		jmp NEXT

; ( a - )
; alternate syntax for dd DROP_ASM
DROP dd $+4
		pop ebx
		jmp NEXT

; ( a b - b )
NIP dd ENTER, SWAP, DROP, EXIT

; ( a b - b a b )
TUCK dd ENTER, SWAP, OVER, EXIT

; >r (a - R: a)
PUSHR dd $+4
		mov [edx], ebx
		add edx, 4
		pop ebx
		jmp NEXT

; r> ( R: a - a )
POPR dd $+4
		push ebx
		sub edx, 4 ; edx point at the top of the stack (which is empty)
							 ; we need to reference the most recently pushed item
		mov ebx, [edx]
		jmp NEXT

; r@ ( R: a - a R: a )
PEEKR dd $+4
		push ebx
		sub edx, 4
		mov ebx, [edx]
		add edx, 4
		jmp NEXT

FOUR db "4   "
TWO db "2   "

init:
		dd DOLITERAL
    dd FOUR
		dd FIND
		dd PRINT
		dd BYE
