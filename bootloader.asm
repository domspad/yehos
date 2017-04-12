; compile with nasm, use as disk image to qemu-system-i386

[BITS 16]
[ORG 0x7c00]

boot_drive equ $

entry:
    cli
    jmp 0x0000:start

    times (8 - $ + entry) db 0   ; pad until boot-info-table

; don't bother with making el torito load the kernel image
iso_boot_info:
bi_pvd  dd 16           ; LBA of primary volume descriptor
bi_file dd 0            ; LBA of boot file
bi_len  dd 0            ; len of boot file
bi_csum dd 0
bi_reserved times 10 dd 0

banner db 10, "SP/OS (2013) Saul Pwanson", 13, 10, 0
errstr db "error loading kernel", 0

; Disk Address Packet
dap db 16, 0            ; [2] sizeof(dap)
    dw 128               ; [2] transfer 16 sectors (before PVB)
    dw 0x8000, 0x0      ; [4] to 0:8000
    dd 1, 0             ; [8] from LBA 0

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00      ; setup stack just before the code

    mov [boot_drive], dl

    ; display banner
    mov si, banner
    call writestr

    ; read sectors from disk
    mov si, dap
    mov dl, [boot_drive]
    mov ah, 0x42
    int 0x13
    jnc readok

readerr:
    mov si, errstr
    call writestr
    hlt

readok:


leap:
    call enable_A20

    lgdt [GDT]                      ; ge
    mov eax, cr0                    ; ro
    or al, 1                        ; ni
    mov cr0, eax                    ; mo
    jmp 0x08:protmain               ; !!

enable_A20: ; from wiki.osdev.org
    call a20wait
    mov al,0xAD
    out 0x64,al

    call a20wait
    mov al,0xD0
    out 0x64,al

    call a20wait2
    in al,0x60
    push eax

    call a20wait
    mov al,0xD1
    out 0x64,al

    call a20wait
    pop eax
    or al,2
    out 0x60,al

    call a20wait
    mov al,0xAE
    out 0x64,al

;  fall-through
;    call a20wait
;    ret

a20wait:
    in al,0x64
    test al,2
    jnz a20wait
    ret

a20wait2:
    in al,0x64
    test al,1
    jz a20wait2
    ret

; character to print in al
putc:
    push ax
    push bx
    mov ah, 0x0e
    mov bx, 0x000f
    int 0x10
    pop bx
    pop ax
    ret

writestr:
    lodsb
    test al, al
    jz end
    call putc
    jmp writestr
end:
    ret

; --- protected mode ---
[BITS 32]

GDT    dw 0x28                      ; limit of 5 entries
       dd GDT                       ; linear address of GDT
       dw 0
        ; 0xBBBBLLLL, 0xBBFLAABB    ; F = GS00b, AA = 1001XDW0
gdtCS  dd 0x0000ffff, 0x00CF9A00    ; 0x08
gdtDS  dd 0x0000ffff, 0x00CF9200    ; 0x10

protmain:
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax

    mov esp, 0x6000      ; data stack grows down

    mov eax, 0x8000
    call eax

_halt:
    hlt
    jmp _halt

    times (512 - $ + entry - 2) db 0 ; pad boot sector with zeroes

       db 0x55, 0xAA ; 2 byte boot signature
