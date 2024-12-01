# Diretórios e arquivos
SRC_DIR = kernel
BUILD_DIR = build
BOOT_SRC = bootloader/boot.asm
ENTRY_SRC = bootloader/entry.asm
KERNEL_SRC = $(SRC_DIR)/kernel.c

BOOT_BIN = $(BUILD_DIR)/boot.bin
ENTRY_OBJ = $(BUILD_DIR)/entry.o
KERNEL_OBJ = $(BUILD_DIR)/kernel.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
SYSTEM_IMG = $(BUILD_DIR)/os-image.bin

all: $(SYSTEM_IMG)

$(BOOT_BIN): $(BOOT_SRC)
	@echo "Compilando bootloader..."
	@nasm -f bin -o $@ $<

$(ENTRY_OBJ): $(ENTRY_SRC)
	@echo "Compilando entry point..."
	@nasm -f elf32 -o $@ $<

$(KERNEL_OBJ): $(KERNEL_SRC)
	@echo "Compilando kernel..."
	@gcc -m32 -ffreestanding -fno-pie -nostdlib -c $< -o $@

$(KERNEL_BIN): $(ENTRY_OBJ) $(KERNEL_OBJ)
	@echo "Linkando kernel..."
	@ld -m elf_i386 -T linker.ld -o $@ $^ --oformat binary

$(SYSTEM_IMG): $(BOOT_BIN) $(KERNEL_BIN)
	@echo "Calculando o número de setores do kernel..."
	@KERNEL_SIZE=$(shell stat -c%s $(KERNEL_BIN)); \
	KERNEL_SECTORS=$$((($$KERNEL_SIZE + 511) / 512)); \
	echo "Número de setores do kernel: $$KERNEL_SECTORS"; \
	echo "Atualizando KERNEL_SECTORS no bootloader..."; \
	sed -i "s/^KERNEL_SECTORS equ .*/KERNEL_SECTORS equ $$KERNEL_SECTORS/" bootloader/boot.asm; \
	echo "Recompilando bootloader com KERNEL_SECTORS atualizado..."; \
	nasm -f bin bootloader/boot.asm -o $(BOOT_BIN)
	@echo "Criando imagem do sistema..."
	@cat $(BOOT_BIN) $(KERNEL_BIN) > $@

clean:
	@echo "Limpando arquivos de build..."
	@rm -rf $(BUILD_DIR)/*

run: all
	@echo "Executando QEMU..."
	@qemu-system-i386 -drive format=raw,file=$(SYSTEM_IMG)

.PHONY: all clean run
