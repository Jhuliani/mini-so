/* kernel/kernel.c */
#include "kernel.h"

// Ponteiro para a memória de vídeo
volatile uint16_t* const video_memory_ptr = (volatile uint16_t*)VIDEO_MEMORY;

// Estado do terminal
size_t terminal_row = 0;
size_t terminal_column = 0;
uint8_t terminal_color = 0x07; // Cinza claro sobre preto
volatile uint16_t* terminal_buffer;

// Buffer de comando
static char cmd_buffer[MAX_CMD_SIZE];
static size_t cmd_index = 0;

// Funções auxiliares
static inline uint8_t make_color(uint8_t fg, uint8_t bg) {
    return fg | (bg << 4);
}

static inline uint16_t make_vgaentry(char c, uint8_t color) {
    return ((uint16_t)color << 8) | (uint16_t)c;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = make_color(VGA_LIGHT_GREY, VGA_BLACK);
    terminal_buffer = video_memory_ptr;

    terminal_clear();
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = make_vgaentry(' ', terminal_color);
        }
    }
}

void terminal_scroll(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t dst_index = y * VGA_WIDTH + x;
            const size_t src_index = (y + 1) * VGA_WIDTH + x;
            terminal_buffer[dst_index] = terminal_buffer[src_index];
        }
    }

    // Limpar a última linha
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        const size_t index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
        terminal_buffer[index] = make_vgaentry(' ', terminal_color);
    }

    if (terminal_row > 0) {
        terminal_row = VGA_HEIGHT - 1;
    }
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    terminal_buffer[index] = make_vgaentry(c, terminal_color);

    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_writestring(const char* data) {
    size_t i = 0;
    while (data[i]) {
        terminal_putchar(data[i++]);
    }
}

// Mapa de teclado (US)
static const char keyboard_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    if (scancode & 0x80) {
        return; // Tecla liberada
    }

    char c = keyboard_map[scancode];
    if (c) {
        if (c == '\n') {
            terminal_putchar('\n');
            cmd_buffer[cmd_index] = '\0';

            // Executar comando
            if (cmd_index > 0) {
                if (strcmp(cmd_buffer, "clear") == 0) {
                    terminal_clear();
                } else {
                    terminal_writestring("Comando desconhecido: ");
                    terminal_writestring(cmd_buffer);
                    terminal_putchar('\n');
                }
            }

            // Resetar buffer de comando
            terminal_writestring("$ ");
            cmd_index = 0;
        }
        else if (c == '\b') {
            if (cmd_index > 0) {
                cmd_index--;
                terminal_putchar('\b');
                terminal_putchar(' ');
                terminal_putchar('\b');
            }
        }
        else if (cmd_index < MAX_CMD_SIZE - 1 && c >= ' ') {
            cmd_buffer[cmd_index++] = c;
            terminal_putchar(c);
        }
    }
}

// Funções de porta I/O
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Função strcmp simplificada
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void kernel_main(void) {
    // Inicializar o terminal
    terminal_initialize();

    terminal_writestring("Kernel inicializado com sucesso!\n");
    terminal_writestring("Sistema Operacional v0.1\n");
    terminal_writestring("$ ");

    // Loop principal
    while (1) {
        keyboard_handler();
    }
}
