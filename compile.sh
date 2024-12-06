#!/bin/bash

set -e

mkdir -p build
rm -f build/*

# Compilar entry point
nasm -f elf32 bootloader/entry.asm -o build/entry.o

# Compilar kernel
gcc -m32 -ffreestanding -fno-pie -nostdlib -c kernel/kernel.c -o build/kernel.o

# Linkar kernel
ld -m elf_i386 -T linker.ld -o build/kernel.bin build/entry.o build/kernel.o --oformat binary

# Compilar bootloader separadamente
nasm -f bin bootloader/boot.asm -o build/boot.bin

# Calcular tamanho do kernel e atualizar bootloader
KERNEL_SIZE=$(stat -c%s build/kernel.bin)
KERNEL_SECTORS=$(( (KERNEL_SIZE + 511) / 512 ))
sed -i "s/^KERNEL_SECTORS equ .*/KERNEL_SECTORS equ $KERNEL_SECTORS/" bootloader/boot.asm

# Recompilar bootloader com novo KERNEL_SECTORS
nasm -f bin bootloader/boot.asm -o build/boot.bin

# Criar imagem final
cat build/boot.bin build/kernel.bin > build/os-image.bin

# Executar QEMU
qemu-system-i386 -fda build/os-image.bin -boot a
