global main
extern c_print_num
extern c_atoi
extern c_compare_strings
extern c_consume_word
extern readline

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

%macro HEADER 1-2 NATIVE_HEADER
%defstr STR_NAME %1
%1_NAME db STR_NAME, 0 ; allocate a variable-length name
%1:
	dd %2            ; address of body
	dd %1_NAME       ; point to the name
	dd PREV_WORD     ; link to prev defn
	dd 1             ; immediate flag
%define PREV_WORD %1
%endmacro


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

HEADER EXIT
		sub R_STACK_PTR, 4
		mov PC, [R_STACK_PTR]
		jmp NEXT

NEXT:
		mov ecx, [PC]
		add PC, 4
		jmp [ecx]

HEADER BYE
		cli
		hlt

HEADER DOLITERAL
		push TOS
		mov TOS, [PC]
		add PC, 4
		jmp NEXT

%define WORD_NAME 4
%define WORD_PREV 8
%define WORD_IMMEDIATE_FLAG 12


; ( word - str|xt 0|-1|+1)
HEADER FIND
		mov eax, [LATEST]
FIND_RECURSIVE:
		push eax 						 ; store top of latest header
		add eax, WORD_NAME
		mov eax, [eax]
		push eax						 ; put ptr to current word name on sys stack
		push TOS						 ; put ptr to input word name on sys stack
		call c_compare_strings
		cmp eax, 0					 ; c_compare_strings returns 0 if the strings are equal
		pop eax
		pop eax							 ; restore top of latest header to eax
		pop eax 						 ; restore eax after function call
		je FIND_NAME_MATCHED
		jmp FIND_NAME_UNMATCHED
FIND_NAME_MATCHED:
		mov TOS, eax ; get rid of name and add xt onto stack
		ppush [eax + WORD_IMMEDIATE_FLAG] ; also push immediate flag onto stack
		jmp NEXT
FIND_NAME_UNMATCHED:
		ppush 0					; We need to compare across registers so we push a zero into TOS
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


HEADER _BL
		ppush ' '
		jmp NEXT

; currently: ( string from input stream - same word on stack )
; eventually: ( delimiter on stack and chars from inputs stream - word on stack )
; Eventually this should create words by delimiting
; a char buffer by the delimiter on the stack.
HEADER _WORD
BREAKPOINT
		mov eax, [INPUT_PTR]
		mov eax, [eax] ; dereference twice to get the value in the input stream.
		and eax, 0x000000FF
		cmp eax, 0		 ; check if we're at the end of the input stream (marked by the null termination character)
		je GET_WORD_FROM_STDIN
		ppush [INPUT_PTR] ; push a pointer to the next thing in the input stream
		push TOS 			 ; set up c call stack
		call c_consume_word
		mov [INPUT_PTR], eax
		pop eax				 ; clear c call stack
		pop eax				 ; clear delimiter from stack
		jmp NEXT
GET_WORD_FROM_STDIN:
		mov eax, INPUT_STREAM
		mov [INPUT_PTR], eax ; reset input pointer to beginning of input stream
		push eax ; give input stream as param to readline
		call readline
		pop eax
		jmp [_WORD]


; ( xt|name 0|1|-1 -> push num onto stack | execute word )
HEADER EXEC_OR_PUSH
		cmp TOS, 0
		je EORP_PUSH_NUM
		jmp EORP_EXECUTE

EORP_PUSH_NUM:
		mov eax, [esp]
		push eax
		call c_atoi ; Assume the string represents a number.
		pop TOS     ; pop esp off
		pop TOS			; Remove the string from the top of the system stack.
		mov TOS, eax ; Replace the string on the top of the forth stack the the result of c_atoi.
		jmp NEXT

EORP_EXECUTE:
		pop TOS ; get rid of flag
		mov eax, [TOS]
		mov ecx, TOS	; ecx must contain the xt of the subroutine
									; this is usually set up by next and is expected
									; by enter
		pop TOS ; get rid of xt
		jmp eax

HEADER DUP
		push TOS
		jmp NEXT

HEADER STAR
		pop eax
		mul TOS
		mov TOS, eax
		jmp NEXT

; ( a b - a b a )
HEADER OVER
		mov eax, [STACK_PTR]
		push TOS
		mov TOS, eax
		jmp NEXT

; ( a b - b a )
HEADER SWAP
		pop eax
		push TOS
		mov TOS, eax
		jmp NEXT

; ( a b c - b c a )
HEADER ROT
		mov eax, TOS
		pop TOS
		push eax

		mov eax, TOS
		add STACK_PTR, 4
		pop TOS ; a is in correct position
		push eax ; c is in correct position
		sub STACK_PTR, 4
		jmp NEXT

; ( a - )
; alternate syntax for dd DROP_ASM
HEADER DROP
		pop TOS
		jmp NEXT

HEADER SQUARED, COMPOSITE_HEADER
		dd DUP, STAR, EXIT

; ( a b - b )
HEADER NIP, COMPOSITE_HEADER
		dd SWAP, DROP, EXIT

; ( a b - b a b )
HEADER TUCK, COMPOSITE_HEADER
		dd SWAP, OVER, EXIT

; >r (a - R: a)
HEADER PUSHR
		mov [edx], TOS
		add edx, 4
		pop TOS
		jmp NEXT

; r> ( R: a - a )
HEADER POPR
		push TOS
		sub edx, 4 ; edx point at the top of the stack (which is empty)
							 ; we need to reference the most recently pushed item
		mov TOS, [edx]
		jmp NEXT

; r@ ( R: a - a R: a )
HEADER PEEKR
		push TOS
		sub edx, 4
		mov TOS, [edx]
		add edx, 4
		jmp NEXT

HEADER PRINT
		push TOS ; move TOS to top of system stack
						 ; so that it will be considered a param by c_print_num
		call c_print_num
		pop TOS  ; get rid of param from system stack
		pop TOS  ; pop the forth stack, print consumes arg
		jmp NEXT

LATEST dd PREV_WORD

HEADER INTERPRET, COMPOSITE_HEADER
		dd _BL, _WORD, FIND, EXEC_OR_PUSH, EXIT

HEADER BRANCH
		mov eax, [PC]
		sub eax, 4
		add PC, eax
		jmp NEXT

HEADER QUIT, COMPOSITE_HEADER
		dd INTERPRET, BRANCH, -4, EXIT

init:
		dd QUIT

; Input stream becomes the buffer where new lines of input are stored.
INPUT_PTR dd INPUT_STREAM
INPUT_STREAM times 256 db 0

RETURN_STACK times 256 db 0
