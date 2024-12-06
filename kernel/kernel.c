#include "kernel.h"

#define MAX_COMMANDS 10
#define CMD_BUFFER_SIZE 256

typedef void (*command_function)(void);

typedef struct {
    const char* name;
    command_function function;
    const char* description;
} command_t;

// Variáveis do terminal
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static char cmd_line[CMD_BUFFER_SIZE];
static size_t cmd_pos = 0;

// Protótipos
static void cmd_help(void);
static void cmd_clear(void);
static void cmd_echo(void);
static void cmd_shutdown(void);

static const command_t commands[] = {
    {"help", cmd_help, "Lista todos os comandos"},
    {"clear", cmd_clear, "Limpa a tela"},
    {"echo", cmd_echo, "Exibe uma mensagem"},
    {"shutdown", cmd_shutdown, "Desliga o sistema"},
    {NULL, NULL, NULL}
};

// Funções de I/O
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value) {
    asm volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Funções de string
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Funções do terminal
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F;
    
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            const size_t index = y * 80 + x;
            VGA_MEMORY[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }
}

void terminal_clear(void) {
    for (size_t y = 0; y < 25; y++) {
        for (size_t x = 0; x < 80; x++) {
            const size_t index = y * 80 + x;
            VGA_MEMORY[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == 25) {
            for (size_t y = 0; y < 24; y++) {
                for (size_t x = 0; x < 80; x++) {
                    VGA_MEMORY[y * 80 + x] = VGA_MEMORY[(y + 1) * 80 + x];
                }
            }
            terminal_row--;
        }
        return;
    }

    const size_t index = terminal_row * 80 + terminal_column;
    VGA_MEMORY[index] = (uint16_t)c | (uint16_t)terminal_color << 8;

    if (++terminal_column == 80) {
        terminal_column = 0;
        if (++terminal_row == 25) {
            for (size_t y = 0; y < 24; y++) {
                for (size_t x = 0; x < 80; x++) {
                    VGA_MEMORY[y * 80 + x] = VGA_MEMORY[(y + 1) * 80 + x];
                }
            }
            terminal_row--;
        }
    }
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; i < strlen(data); i++)
        terminal_putchar(data[i]);
}

void terminal_backspace(void) {
    if (terminal_column > 0) {
        terminal_column--;
        const size_t index = terminal_row * 80 + terminal_column;
        VGA_MEMORY[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
}

// Comandos
void cmd_help(void) {
    terminal_writestring("Comandos disponíveis:\n");
    for(int i = 0; commands[i].name != NULL; i++) {
        terminal_writestring(commands[i].name);
        terminal_writestring(" - ");
        terminal_writestring(commands[i].description);
        terminal_writestring("\n");
    }
}

void cmd_clear(void) {
    terminal_clear();
}

void cmd_echo(void) {
    char* arg = cmd_line;
    while(*arg && *arg != ' ') arg++;
    if(*arg) arg++;
    terminal_writestring(arg);
    terminal_writestring("\n");
}

void cmd_shutdown(void) {
    outw(0x604, 0x2000);
}

// Teclado
static const char scancode_to_char[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

char keyboard_getchar(void) {
    char c = 0;
    while (c == 0) {
        if ((inb(0x64) & 1) != 0) {
            uint8_t scancode = inb(0x60);
            if (scancode < 128) {
                c = scancode_to_char[scancode];
            }
        }
    }
    return c;
}

void execute_command(void) {
    char* cmd = cmd_line;
    for(int i = 0; commands[i].name != NULL; i++) {
        size_t len = strlen(commands[i].name);
        if(strncmp(cmd, commands[i].name, len) == 0) {
            commands[i].function();
            return;
        }
    }
    terminal_writestring("Comando não encontrado: ");
    terminal_writestring(cmd);
    terminal_writestring("\n");
}

void kernel_main(void) {
    terminal_initialize();
    terminal_writestring("Sistema Operacional v0.1\n");
    terminal_writestring("Digite 'help' para ver os comandos\n");
    terminal_writestring("$ ");

    while(1) {
        char c = keyboard_getchar();
        if(c == '\n') {
            cmd_line[cmd_pos] = '\0';
            terminal_writestring("\n");
            if(cmd_pos > 0) {
                execute_command();
            }
            cmd_pos = 0;
            terminal_writestring("$ ");
        }
        else if(c == '\b' && cmd_pos > 0) {
            cmd_pos--;
            terminal_backspace();
        }
        else if(c >= ' ' && cmd_pos < CMD_BUFFER_SIZE - 1) {
            cmd_line[cmd_pos++] = c;
            terminal_putchar(c);
        }
    }
}