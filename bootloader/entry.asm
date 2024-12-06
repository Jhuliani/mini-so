[BITS 32]
[GLOBAL loader_entry]
[EXTERN kernel_main]

section .text
align 4
loader_entry:
    ; Configurar stack
    mov esp, stack_top
    
    ; Chamar kernel
    call kernel_main

    ; Loop infinito
    cli
    hlt
    jmp $

section .bss
align 4
stack_bottom:
    resb 16384  ; 16 KB para stack
stack_top: