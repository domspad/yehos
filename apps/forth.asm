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

%define HEADER_SIZE 16

%define PREV_WORD 0

%define NATIVE_HEADER $+HEADER_SIZE

%define COMPOSITE_HEADER ENTER

%macro HEADER 2-3 NATIVE_HEADER
		dd %3            ; address of body
		dd %1            ; name
		dd PREV_WORD     ; link to prev defn
		dd 0             ; immediate flag
%define PREV_WORD %2
%endmacro

ENTER:
		mov [ebp], esi
		add ebp, 4
		add ecx, HEADER_SIZE
		mov esi, ecx
		jmp NEXT

EXIT:
		HEADER "EXIT", EXIT
		sub ebp, 4
		mov esi, [ebp]
		jmp NEXT

NEXT:
		mov ecx, [esi]
		add esi, 4
		jmp [ecx]

BYE:
		HEADER "BYE ", BYE
		cli
		hlt

DOLITERAL:
		HEADER "DOLI", DOLITERAL
		push ebx
		mov ebx, [esi]
		add esi, 4
		jmp NEXT

PRINT:
		HEADER "PRIN", PRINT
		push ebx ; move TOS to top of system stack
						 ; so that it will be considered a param by c_print num
		call c_print_num
		pop ebx  ; get rid of param from system stack
		pop ebx  ; pop the forth stack, print consumes arg
		jmp NEXT

; ( word - addr )
FIND:
		HEADER "FIND", FIND
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
		HEADER "BLAN", BLANK
		push ebx
		mov ebx, 0
		jmp NEXT

; Eventually this should create words by delimiting
; a char buffer by the delimiter on the stack.
FWORD:
		HEADER "WORD", FWORD
		mov ebx, [INPUT_PTR]
		jmp NEXT

; ( 0 | addr -> num | execution ) 
EXEC_OR_PUSH:
		HEADER "EORP", EXEC_OR_PUSH
		cmp ebx, 0
		je ASM_PUSH_NUM
		jmp ASM_EXECUTE
ASM_PUSH_NUM:
		pop ebx
ASM_EXECUTE:
		jmp [ebx]


DUP:
		HEADER "DUPL", DUP
		push ebx
		jmp NEXT

STAR:
		HEADER "STAR", STAR
		pop eax
		mul ebx
		mov ebx, eax
		jmp NEXT

; ( a b - a b a )
OVER:
		HEADER "OVER", OVER
		mov eax, [esp]
		push ebx
		mov ebx, eax
		jmp NEXT

; ( a b - b a )
SWAP:
		HEADER "SWAP", SWAP
		pop eax
		push ebx
		mov ebx, eax
		jmp NEXT

; ( a b c - b c a )
ROT:
		HEADER "ROTA", ROT
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
DROP:
		HEADER "DROP", DROP
		pop ebx
		jmp NEXT

SQUARED:
		HEADER "SQUA", SQUARED, COMPOSITE_HEADER
		dd DUP, STAR, EXIT

; ( a b - b )
NIP:
		HEADER "NIPP", NIP, COMPOSITE_HEADER
		dd SWAP, DROP, EXIT

; ( a b - b a b )
TUCK:
		HEADER "TUCK", TUCK, COMPOSITE_HEADER
		dd SWAP, OVER, EXIT

; >r (a - R: a)
PUSHR:
		HEADER "PUSR", PUSHR
		mov [edx], ebx
		add edx, 4
		pop ebx
		jmp NEXT

; r> ( R: a - a )
POPR:
		HEADER "POPR", POPR
		push ebx
		sub edx, 4 ; edx point at the top of the stack (which is empty)
							 ; we need to reference the most recently pushed item
		mov ebx, [edx]
		jmp NEXT

; r@ ( R: a - a R: a )
PEEKR:
		HEADER "PEKR", PEEKR
		push ebx
		sub edx, 4
		mov ebx, [edx]
		add edx, 4
		jmp NEXT

DICT dd PREV_WORD

INTERPRET:
		dd ENTER, BLANK, FWORD, FIND, EXEC_OR_PUSH, EXIT

init:
		dd DOLITERAL
		dd 42
		dd DOLITERAL
		dd 123
		dd NIP
		dd PRINT
		dd DOLITERAL
		dd "DOLI"
		dd FIND
		dd PRINT
		dd BYE
