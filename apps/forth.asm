global main
extern c_print_num
extern c_atoi
extern c_compare_strings
extern c_consume_word
extern c_strcpy
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
%define IMMEDIATE -1

%macro HEADER 1-3 NATIVE_HEADER, 1
%defstr STR_NAME %1
%1_NAME db STR_NAME, 0 ; allocate a variable-length name
%1:
	dd %2            ; address of body
	dd %1_NAME       ; point to the name
	dd PREV_WORD     ; link to prev defn
	dd %3            ; immediate flag
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


; ( str num -> num onto stack )
HEADER PUSHNUM
		mov eax, [esp]
		push eax
		push TOS
		call c_atoi ; Assume the string represents a number.
		pop TOS     ; pop esp off
		pop TOS			; Remove the string from the top of the system stack.
		mov TOS, eax ; Replace the string on the top of the forth stack the the result of c_atoi.
		jmp NEXT

; ( xt -> execute token )
HEADER EXECUTE
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

HEADER LT0
		cmp TOS, 0
		jl push1
push0:
		mov TOS, 0
		jmp NEXT
push1:
		mov TOS, 1
		jmp NEXT

HEADER BRANCH
		mov eax, [PC]
		sub eax, 4
		add PC, eax
		jmp NEXT

; ( -1 | 0 | +1 , (next val in instream) - execute next instruction or (val/4) instructions ahead )
; jumps ahead if there's a 0 on the stack
HEADER QBRANCH
		cmp TOS, 0
		pop TOS ; clears flag
		je jump_using_instream
continue_ahead:
		add PC, 4
		jmp NEXT
jump_using_instream:
		mov eax, [PC]
		sub eax, 4
		add PC, eax
		jmp NEXT

; ( 0 | a -- 0 | (a, a) )
HEADER ?DUP, COMPOSITE_HEADER
		dd DUP, QBRANCH, 12, DUP, EXIT

; ( -- 0 | 1 )
; pushes 1 if compile mode else 0
STATE_VAR dd 0
HEADER STATE
		ppush STATE_VAR
		jmp NEXT

; ( addr -- val )
HEADER @
		mov TOS, [TOS]
		jmp NEXT

; set interpret state
HEADER LEFT_BRACKET, NATIVE_HEADER, IMMEDIATE
		mov eax, 0
		mov [STATE_VAR], eax
		jmp NEXT

; set compile state
HEADER RIGHT_BRACKET, NATIVE_HEADER, IMMEDIATE
		mov eax, 1
		mov [STATE_VAR], eax
		jmp NEXT

HEADER COLON, COMPOSITE_HEADER
		dd CREATE, RIGHT_BRACKET, EXIT

HEADER SEMICOLON, COMPOSITE_HEADER, IMMEDIATE
		dd BRACKET_TICK, EXIT, COMPILE, EXIT

HEADER BRACKET_TICK
		dd EXIT

; ( "<name>" -- )
; creates a dictionary entry for <name>
HEADER CREATE, COMPOSITE_HEADER
		dd _BL, _WORD, CREATE_FROM_STACK, EXIT

; ( str -- )
; takes a ptr to a name (str) from the top of the stack
; and creates a dictionary header pointing that that name
HEADER CREATE_FROM_STACK
		; store the string above the dictionary entry
		push TOS									; move word ptr from top of 4th stack to top of c param stack
		mov ecx, [HERE]						; store the addr of the str_name
		push ecx									; push eod ptr to c param stack
		call c_strcpy							; copy word from input stream to eod
															; strlen is in eax after return
		pop TOS
		pop TOS										; restore the stack

		; create the header
		add eax, [HERE]						; eax held the return value of c_strcpy - the length of the new word name
															; we add [HERE] so it holds the new addr of the end of the dictionary
		mov edx, ENTER
		mov [eax], edx
		add eax, 4
		mov edx, [HERE]
		mov [eax], edx						; ptr to string name
		add eax, 4
		mov ecx, [LATEST]					; xt of the previous word
		mov [eax], ecx
		sub eax, 8
		mov [LATEST], eax					; xt of the current word is the new value of latest
		add eax, 8								; move eax back to the end of the dictionary
		add eax, 4
		mov edx, 1
		mov [eax], edx						; immediate flag defaults to 1
		add eax, 4

		; set the new end of dictionary
		mov [HERE], eax
		jmp NEXT

; compiles a doliteral along with the number to be pushed
HEADER LITERAL, COMPOSITE_HEADER
		dd EXIT

; ( addr -- )
; adds the address of a word to the end of the dictionary
HEADER COMPILE
		mov eax, [HERE]
		mov [eax], TOS
		pop TOS												; clear the XT off the stack
		add eax, 4
		mov [HERE], eax	; advance the eod ptr
		jmp NEXT

; execute is 1 else compile
; ( -1 | 1 )
HEADER EXEC_OR_COMPILE, COMPOSITE_HEADER
		dd LT0, QBRANCH, 20, EXECUTE, BRANCH, 12, COMPILE, EXIT

HEADER INTERPRET_WORD, COMPOSITE_HEADER
		dd _BL, _WORD, FIND, QBRANCH, 20, EXECUTE, BRANCH, 12, PUSHNUM, EXIT

HEADER COMPILE_WORD, COMPOSITE_HEADER
		dd _BL, _WORD, FIND, ?DUP, QBRANCH, 20, EXEC_OR_COMPILE, BRANCH, 16, PUSHNUM, LITERAL, EXIT

HEADER QUIT, COMPOSITE_HEADER
		dd COMPILE_OR_INTERPRET, BRANCH, -4, EXIT

HEADER COMPILE_OR_INTERPRET, COMPOSITE_HEADER
		dd STATE, @, QBRANCH, 20, COMPILE_WORD, BRANCH, 12, INTERPRET_WORD, EXIT

LATEST dd PREV_WORD

init:
		dd QUIT

; Input stream becomes the buffer where new lines of input are stored.
INPUT_PTR dd INPUT_STREAM
INPUT_STREAM times 256 db 0

RETURN_STACK times 256 db 0

; holds the address of the end of dictionary
HERE dd HERE+4
times 1024 db 0
