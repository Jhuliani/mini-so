#include "kernel.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

static uint16_t* const vga_buffer = (uint16_t*)VGA_MEMORY;
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

typedef struct {
    const char* name;
    void (*function)(void);
    const char* description;
} command_t;

static void cmd_help(void);
static void cmd_calc(void);
static void scroll_terminal(void);
static void gotoxy(int x, int y);

static const command_t commands[] = {
    {"calc", cmd_calc, "Calculadora simples"},
    {"clear", terminal_clear, "Limpa a tela"},
    {"help", cmd_help, "Mostra comandos"},
    {NULL, NULL, NULL}
};

static inline void outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = VGA_WHITE;
    terminal_clear();
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            vga_buffer[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

static void scroll_terminal(void) {
    for (size_t y = 0; y < VGA_HEIGHT - 2; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    terminal_row--;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT - 1) {
            scroll_terminal();
        }
        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = (uint16_t)c | (uint16_t)terminal_color << 8;

    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT - 1) {
            scroll_terminal();
        }
    }
}

void terminal_writestring(const char* str) {
    for (size_t i = 0; str[i]; i++) {
        terminal_putchar(str[i]);
    }
}

void terminal_backspace(void) {
    if (terminal_column > 0) {
        terminal_column--;
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        vga_buffer[index] = (uint16_t)' ' | (uint16_t)terminal_color << 8;
    }
}

static void gotoxy(int x, int y) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        terminal_column = x;
        terminal_row = y;
    }
}

void update_statusbar(void) {
    size_t old_row = terminal_row;
    size_t old_col = terminal_column;
    uint8_t old_color = terminal_color;
    
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
    terminal_color = VGA_LIGHT_GREY | (VGA_BLUE << 4);
    
    for (size_t i = 0; i < VGA_WIDTH; i++) {
        terminal_putchar(' ');
    }
    
    terminal_row = VGA_HEIGHT - 1;
    terminal_column = 0;
    
    outb(0x70, 0x04);
    uint8_t hour = inb(0x71);
    outb(0x70, 0x02);
    uint8_t min = inb(0x71);
    
    hour = ((hour / 16) * 10) + (hour & 0x0F);
    min = ((min / 16) * 10) + (min & 0x0F);
    
    char buf[32];
    buf[0] = ' ';
    buf[1] = '0' + (hour / 10);
    buf[2] = '0' + (hour % 10);
    buf[3] = ':';
    buf[4] = '0' + (min / 10);
    buf[5] = '0' + (min % 10);
    buf[6] = 0;
    
    terminal_writestring(buf);
    terminal_writestring(" | RAM: 640KB | SO Grupo 08");
    
    terminal_row = old_row;
    terminal_column = old_col;
    terminal_color = old_color;
}

char keyboard_getchar(void) {
    static const char scancode_map[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };
    
    static const char numpad_map[16] = {
        '7', '8', '9', '-',
        '4', '5', '6', '+',
        '1', '2', '3', '0',
        '.', '*', '/', '\n'
    };

    static bool shift_pressed = false;

    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0x2A || scancode == 0x36) { // Left or right Shift pressed
                shift_pressed = true;
                continue;
            }
            if (scancode == 0xAA || scancode == 0xB6) { // Left or right Shift released
                shift_pressed = false;
                continue;
            }
            
            // Handle numpad
            if (scancode >= 0x47 && scancode <= 0x53) {
                char c = numpad_map[scancode - 0x47];
                if (c) return c;
                continue;
            }

            // Handle shift + number for symbols
            if (shift_pressed && scancode >= 0x02 && scancode <= 0x0D) {
                static const char shift_nums[] = "!@#$%^&*()_+";
                return shift_nums[scancode - 0x02];
            }

            // Handle normal keys
            if (scancode < 128) {
                char c = scancode_map[scancode];
                if (c) return c;
            }
        }
    }
}

static void cmd_help(void) {
    terminal_writestring("Comandos disponiveis:\n");
    for (const command_t* cmd = commands; cmd->name; cmd++) {
        terminal_writestring(cmd->name);
        terminal_writestring(" - ");
        terminal_writestring(cmd->description);
        terminal_writestring("\n");
    }
}

static void cmd_calc(void) {
    terminal_writestring("Calculadora Simples\n");
    terminal_writestring("Use: numero operacao numero\n");
    terminal_writestring("Operacoes: +, -, *, /\n");
    
    int num1 = 0, num2 = 0;
    char op = 0;
    bool reading_second = false;
    
    while (1) {
        char c = keyboard_getchar();
        
        if (c >= '0' && c <= '9') {
            int* num = reading_second ? &num2 : &num1;
            *num = (*num * 10) + (c - '0');
            terminal_putchar(c);
        }
        else if ((c == '+' || c == '-' || c == '*' || c == 'x' || c == '/') && !reading_second) {
            op = (c == 'x') ? '*' : c;
            reading_second = true;
            terminal_putchar(c);
        }
        else if (c == '\n') {
            terminal_putchar('\n');
            break;
        }
    }
    
    int result = 0;
    switch (op) {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '*': result = num1 * num2; break;
        case '/': result = num2 ? num1 / num2 : 0; break;
        default: terminal_writestring("Operador invalido\n"); return;
    }
    
    terminal_writestring("Resultado: ");
    
    if (result < 0) {
        terminal_putchar('-');
        result = -result;
    }
    
    char buf[16];
    int idx = 0;
    do {
        buf[idx++] = '0' + (result % 10);
        result /= 10;
    } while (result > 0);
    
    while (idx > 0) {
        terminal_putchar(buf[--idx]);
    }
    terminal_putchar('\n');
}

void kernel_main(void) {
    terminal_initialize();
    terminal_writestring("Sistema Operacional Grupo 08\n");
    terminal_writestring("Digite 'help' para ver os comandos\n");
    update_statusbar();
    
    while (1) {
        terminal_writestring("$ ");
        char input[CMD_BUFFER_SIZE] = {0};
        size_t pos = 0;
        
        while (pos < CMD_BUFFER_SIZE - 1) {
            char c = keyboard_getchar();
            if (c == '\n') {
                terminal_putchar(c);
                break;
            }
            if (c == '\b' && pos > 0) {
                pos--;
                terminal_backspace();
            } else if (c >= ' ') {
                input[pos++] = c;
                terminal_putchar(c);
            }
        }
        
        input[pos] = '\0';
        
        if (pos > 0) {
            bool found = false;
            for (const command_t* cmd = commands; cmd->name; cmd++) {
                size_t len;
                for (len = 0; cmd->name[len]; len++);
                
                bool match = true;
                for (size_t i = 0; i < len; i++) {
                    if (input[i] != cmd->name[i]) {
                        match = false;
                        break;
                    }
                }
                
                if (match && input[len] == '\0') {
                    cmd->function();
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                terminal_writestring("Comando nao encontrado\n");
            }
        }
        
        update_statusbar();
    }
}