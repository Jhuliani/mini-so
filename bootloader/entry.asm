[BITS 32]
[GLOBAL loader_entry]
extern kernel_main

section .text
loader_entry:
    push ebp
    mov ebp, esp
    call kernel_main   ; Chama a função kernel_main do kernel.c
    mov esp, ebp
    pop ebp
    ret
