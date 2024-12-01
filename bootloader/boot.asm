[BITS 16]
[ORG 0x7C00]

KERNEL_OFFSET equ 0x1000
KERNEL_SECTORS equ 4

start:
    ; Configurar segmentos
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Salvar drive de boot
    mov [BOOT_DRIVE], dl

    ; Mensagens iniciais
    mov si, MSG_START
    call print_string

    ; Resetar o sistema de disco
    mov ah, 0
    mov dl, [BOOT_DRIVE]  ; Usar o drive de boot
    int 0x13
    jc disk_error

    ; Carregar o kernel
    mov si, MSG_LOAD
    call print_string

    ; Configurar a leitura do disco
    mov bx, KERNEL_OFFSET  ; Endereço onde carregar
    mov ah, 0x02           ; Função de leitura
    mov al, KERNEL_SECTORS ; Quantidade de setores
    mov ch, 0              ; Cilindro 0
    mov cl, 2              ; Começar do setor 2
    mov dh, 0              ; Cabeça 0
    mov dl, [BOOT_DRIVE]   ; Usar o drive número fornecido pela BIOS

    ; Mensagem de depuração antes de ler
    mov si, MSG_BEFORE_READ
    call print_string

    ; Tentar ler o disco
    int 0x13
    jc disk_error          ; Se houve erro, mostrar mensagem

    ; Mensagem de depuração após ler
    mov si, MSG_AFTER_READ
    call print_string

    ; Sucesso na leitura
    mov si, MSG_OK
    call print_string

    ; Mudar para modo protegido
    cli                    ; Desabilitar interrupções
    lgdt [gdt_descriptor]  ; Carregar GDT
    mov eax, cr0
    or eax, 0x1            ; Ativar modo protegido
    mov cr0, eax
    jmp CODE_SEG:init_pm   ; Salto far para atualizar CS

disk_error:
    mov si, DISK_ERROR
    call print_string

    ; Mostrar código de erro retornado em AH
    mov ah, 0x0E
    mov al, ' '
    int 0x10
    mov al, '0'
    int 0x10
    mov al, 'x'
    int 0x10
    mov al, ah
    call print_hex

    jmp $                  ; Loop infinito

print_string:
    mov ah, 0x0E           ; Função de teletype
.loop:
    lodsb                  ; Carregar próximo caractere
    test al, al            ; Verificar se é zero
    jz .done               ; Se for zero, terminou
    int 0x10               ; Caso contrário, imprimir
    jmp .loop              ; E continuar
.done:
    ret

print_hex:
    ; Converte AL para dois dígitos hexadecimais e imprime
    push ax
    mov cl, 4
    call print_hex_digit
    pop ax
    call print_hex_digit
    ret

print_hex_digit:
    rol al, cl
    and al, 0x0F
    cmp al, 0x0A
    jl .digit
    add al, 'A' - 0x0A
    jmp .print
.digit:
    add al, '0'
.print:
    mov ah, 0x0E
    int 0x10
    ret

; Definições da GDT
gdt_start:
    dq 0x0000000000000000      ; Descritor nulo

gdt_code:                      ; Descritor do segmento de código
    dw 0xFFFF                  ; Limite (0-15)
    dw 0x0000                  ; Base (0-15)
    db 0x00                    ; Base (16-23)
    db 10011010b               ; Access byte
    db 11001111b               ; Flags e limite (16-19)
    db 0x00                    ; Base (24-31)

gdt_data:                      ; Descritor do segmento de dados
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Tamanho da GDT menos 1
    dd gdt_start               ; Endereço da GDT

; Constantes de segmentos
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

[BITS 32]
init_pm:
    ; Atualizar segmentos
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Configurar stack
    mov esp, 0x90000

    ; Mensagem de depuração
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F

    ; Pular para o kernel
    jmp CODE_SEG:KERNEL_OFFSET   ; Salto far para o endereço onde o kernel foi carregado

    ; Se retornar, travar
    jmp $

; Dados
BOOT_DRIVE db 0
MSG_START      db "Bootloader iniciado", 13, 10, 0
MSG_LOAD       db "Carregando kernel...", 13, 10, 0
MSG_BEFORE_READ db "Antes de ler o disco.", 13, 10, 0
MSG_AFTER_READ db "Após ler o disco.", 13, 10, 0
MSG_OK         db "Kernel carregado!", 13, 10, 0
DISK_ERROR     db "Erro ao ler disco! Codigo:", 0

; Padding e assinatura
times 510-($-$$) db 0
dw 0xAA55
