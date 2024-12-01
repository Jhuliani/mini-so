#!/bin/bash

# Criar diretório build se não existir
mkdir -p build

echo "Limpando build anterior..."
rm -f build/*

echo "Compilando bootloader..."
nasm -f bin bootloader/boot.asm -o build/boot.bin
if [ $? -ne 0 ]; then
    echo "Erro ao compilar bootloader!"
    exit 1
fi

echo "Compilando entry point..."
nasm -f elf32 bootloader/entry.asm -o build/entry.o
if [ $? -ne 0 ]; then
    echo "Erro ao compilar entry point!"
    exit 1
fi

echo "Compilando kernel..."
gcc -m32 -ffreestanding -fno-pie -nostdlib -c kernel/kernel.c -o build/kernel.o
if [ $? -ne 0 ]; then
    echo "Erro ao compilar kernel!"
    exit 1
fi

echo "Linkando kernel..."
ld -m elf_i386 -T linker.ld -o build/kernel.bin build/entry.o build/kernel.o --oformat binary
if [ $? -ne 0 ]; then
    echo "Erro ao linkar kernel!"
    exit 1
fi

echo "Calculando o número de setores do kernel..."
KERNEL_SIZE=$(stat -c%s build/kernel.bin)
KERNEL_SECTORS=$((($KERNEL_SIZE + 511) / 512))
echo "Número de setores do kernel: $KERNEL_SECTORS"

echo "Atualizando KERNEL_SECTORS no bootloader..."
sed -i "s/^KERNEL_SECTORS equ .*/KERNEL_SECTORS equ $KERNEL_SECTORS/" bootloader/boot.asm

echo "Recompilando bootloader com KERNEL_SECTORS atualizado..."
nasm -f bin bootloader/boot.asm -o build/boot.bin
if [ $? -ne 0 ]; then
    echo "Erro ao recompilar bootloader!"
    exit 1
fi

echo "Criando imagem do sistema..."
cat build/boot.bin build/kernel.bin > build/os-image.bin

echo "Executando QEMU..."
qemu-system-i386 -fda build/os-image.bin -boot a
