[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000
KERNEL_SECTORS equ 11

start:
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    mov si, MSG_LOAD
    call print_string

    mov ah, 0x02              ; Ler setores
    mov al, KERNEL_SECTORS    ; Número de setores
    mov ch, 0                 ; Cilindro 0
    mov cl, 2                 ; Setor 2
    mov dh, 0                 ; Cabeça 0
    mov dl, 0x00              ; Drive A
    mov bx, KERNEL_OFFSET     ; Endereço de carregamento
    int 0x13                  ; Interrupção do BIOS
    jc error

    mov si, MSG_OK
    call print_string

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:protected_mode

error:
    mov si, MSG_ERROR
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

MSG_LOAD db "Carregando kernel...", 13, 10, 0
MSG_OK db "Kernel carregado!", 13, 10, 0
MSG_ERROR db "Erro ao carregar!", 13, 10, 0

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

gdt_start:
    dd 0, 0
    db 0xFF, 0xFF, 0, 0, 0, 10011010b, 11001111b, 0
    db 0xFF, 0xFF, 0, 0, 0, 10010010b, 11001111b, 0
gdt_end:

[BITS 32]
protected_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    jmp KERNEL_OFFSET

times 510-($-$$) db 0
dw 0xAA55
