global main
extern c_print_num

%define TOS ebx						; holds the top value of the forth param stack.
%define STACK_PTR esp 		; points to the top of the system stack,
													; which contains the rest of the forth param stack.
%define PC esi						; forth program counter
%define R_STACK_PTR ebp		; point to the top of the return stack.

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

RETURN_STACK:
		dd 0x0
		dd 0x0
		dd 0x0
		dd 0x0

main:
		mov R_STACK_PTR, RETURN_STACK
		mov PC, init
		jmp NEXT

ENTER:
		mov [R_STACK_PTR], PC
		add R_STACK_PTR, 4
		add ecx, HEADER_SIZE
		mov PC, ecx
		jmp NEXT

EXIT:
		HEADER "EXIT", EXIT
		sub R_STACK_PTR, 4
		mov PC, [R_STACK_PTR]
		jmp NEXT

NEXT:
		mov ecx, [PC]
		add PC, 4
		jmp [ecx]

BYE:
		HEADER "BYE ", BYE
		cli
		hlt

DOLITERAL:
		HEADER "DOLI", DOLITERAL
		push TOS
		mov TOS, [PC]
		add PC, 4
		jmp NEXT

PRINT:
		HEADER "PRIN", PRINT
		push TOS ; move TOS to top of system stack
						 ; so that it will be considered a param by c_print num
		call c_print_num
		pop TOS  ; get rid of param from system stack
		pop TOS  ; pop the forth stack, print consumes arg
		jmp NEXT

; ( word - addr )
FIND:
		HEADER "FIND", FIND
		mov eax, [DICT]
ASM_FIND_RECURSIVE:
		add eax, 4
		push TOS
		mov TOS, 0
		cmp [eax], TOS
		pop TOS
		je ASM_WORD_NOT_FOUND
		cmp [eax], TOS
		je ASM_FIND_MATCH_FOUND
		; not found match
		add eax, 4 ; EAX now points to the address of the previous word in the dictionary.
		mov eax, [eax]
		jmp ASM_FIND_RECURSIVE
		; match found
ASM_FIND_MATCH_FOUND:
		push TOS
		sub eax, 4
		mov TOS, eax
		jmp NEXT
ASM_WORD_NOT_FOUND:
		push TOS
		mov TOS, 0
		jmp NEXT

BLANK:
		HEADER "BLAN", BLANK
		push TOS
		mov TOS, 0
		jmp NEXT

; Eventually this should create words by delimiting
; a char buffer by the delimiter on the stack.
FWORD:
		HEADER "WORD", FWORD
		mov TOS, [INPUT_PTR]
		jmp NEXT

; ( 0 | addr -> num | execution ) 
EXEC_OR_PUSH:
		HEADER "EORP", EXEC_OR_PUSH
		cmp TOS, 0
		je ASM_PUSH_NUM
		jmp ASM_EXECUTE
ASM_PUSH_NUM:
		pop TOS
ASM_EXECUTE:
		jmp [TOS]


DUP:
		HEADER "DUPL", DUP
		push TOS
		jmp NEXT

STAR:
		HEADER "STAR", STAR
		pop eax
		mul TOS
		mov TOS, eax
		jmp NEXT

; ( a b - a b a )
OVER:
		HEADER "OVER", OVER
		mov eax, [STACK_PTR]
		push TOS
		mov TOS, eax
		jmp NEXT

; ( a b - b a )
SWAP:
		HEADER "SWAP", SWAP
		pop eax
		push TOS
		mov TOS, eax
		jmp NEXT

; ( a b c - b c a )
ROT:
		HEADER "ROTA", ROT
		mov eax, TOS
		pop TOS
		push eax

		mov eax, TOS
		add STACK_PTR, 4
		pop TOS ; a is correct position
		push eax ; c is in correct position
		sub STACK_PTR, 4
		jmp NEXT

; ( a - )
; alternate syntax for dd DROP_ASM
DROP:
		HEADER "DROP", DROP
		pop TOS
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
		mov [edx], TOS
		add edx, 4
		pop TOS
		jmp NEXT

; r> ( R: a - a )
POPR:
		HEADER "POPR", POPR
		push TOS
		sub edx, 4 ; edx point at the top of the stack (which is empty)
							 ; we need to reference the most recently pushed item
		mov TOS, [edx]
		jmp NEXT

; r@ ( R: a - a R: a )
PEEKR:
		HEADER "PEKR", PEEKR
		push TOS
		sub edx, 4
		mov TOS, [edx]
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
