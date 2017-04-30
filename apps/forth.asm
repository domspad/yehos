global main
extern c_print_num

%define TOS ebx						; holds the top value of the forth param stack.
%define STACK_PTR esp 		; points to the top of the system stack,
													; which contains the rest of the forth param stack.
%define PC esi						; forth program counter
%define R_STACK_PTR ebp		; point to the top of the return stack.

%macro ppush 1
		push TOS
		mov TOS, %1
%endmacro

%define HEADER_SIZE 16
%define PREV_WORD 0
%define NATIVE_HEADER $+HEADER_SIZE
%define COMPOSITE_HEADER ENTER

%macro HEADER 2-3 NATIVE_HEADER
dd %3            ; address of body
dd %1            ; name
dd PREV_WORD     ; link to prev defn
dd 1             ; immediate flag
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

%define WORD_NAME 4
%define WORD_PREV 8
%define WORD_IMMEDIATE_FLAG 12

; ( word - str|xt 0|-1|+1)
FIND:
		HEADER "FIND", FIND
		mov eax, [LATEST]
FIND_RECURSIVE:
		cmp [eax + WORD_NAME], TOS
		je FIND_NAME_MATCHED
		jmp FIND_NAME_UNMATCHED

FIND_NAME_MATCHED:
		mov TOS, eax ; get rid of name and add xt onto stack
		ppush [eax + WORD_IMMEDIATE_FLAG] ; also push immediate flag onto stack
		jmp NEXT

FIND_NAME_UNMATCHED:
		ppush 0					; We need to compare across registers,
										; so we push a zero into TOS
		cmp [eax + WORD_PREV], TOS  ; see if it is the last word in the dictionary
		pop TOS
		je FIND_WORD_NOT_FOUND
		jmp FIND_NEXT_WORD

FIND_WORD_NOT_FOUND:
    ppush 0
		jmp NEXT

FIND_NEXT_WORD:
		mov eax, [eax + WORD_PREV] ; go to previous word
		jmp FIND_RECURSIVE


BLANK:
		HEADER "BLAN", BLANK
		ppush 0
		jmp NEXT

; currently: ( string from input stream - same word on stack )
; eventually: ( delimiter on stack and chars from inputs stream - word on stack )
; Eventually this should create words by delimiting
; a char buffer by the delimiter on the stack.
FWORD:
		HEADER "WORD", FWORD
		mov TOS, [INPUT_PTR]
		mov TOS, [TOS] ; dereference twice to get the value in the input stream.
		mov eax, 4
		add [INPUT_PTR], eax
		jmp NEXT

; ( xt|name 0|1|-1 -> push num onto stack | execute word )
EXEC_OR_PUSH:
		HEADER "EORP", EXEC_OR_PUSH
		cmp TOS, 0
		je EORP_PUSH_NUM
		jmp EORP_EXECUTE

EORP_PUSH_NUM:
		pop TOS
		jmp NEXT

EORP_EXECUTE:
		pop TOS ; get rid of flag
		mov eax, [TOS]
		mov ecx, TOS	; ecx must contain the xt of the subroutine
									; this is usually set up by next and is expected
									; by enter
		pop TOS ; get rid of xt
		jmp eax

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

LATEST dd PREV_WORD

INTERPRET:
		HEADER "INTR", INTERPRET, COMPOSITE_HEADER
		dd BLANK, FWORD, FIND, EXEC_OR_PUSH, EXIT

init:
		dd INTERPRET
		dd INTERPRET
		dd INTERPRET
		dd BYE

INPUT_STREAM dd 42, "SQUA", "PRIN"

INPUT_PTR dd INPUT_STREAM
