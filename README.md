# Sistema Operacional Simples

Este é um projeto de sistema operacional básico para fins educativos. Ele consiste em um bootloader em Assembly, um ponto de entrada em Assembly e um kernel básico escrito em C.

## Estrutura do Projeto

- **bootloader/boot.asm**: Bootloader em Assembly.
- **bootloader/entry.asm**: Ponto de entrada para o kernel.
- **kernel/kernel.c**: Código principal do kernel.
- **kernel/kernel.h**: Cabeçalho do kernel.
- **Makefile**: Automação de build e execução.

## Como Compilar e Executar

1. entre no diretorio do projeoto e rode o './compile.sh'

## Requisitos

- `gcc`, `nasm`, `ld`, e `qemu-system-x86_64`.
