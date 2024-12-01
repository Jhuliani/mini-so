// Definições
#define VIDEO_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_CMD_SIZE 256

// Cores
#define BLACK 0
#define LIGHT_GREY 7
#define WHITE 15
#define MAKE_COLOR(fg, bg) ((bg << 4) | fg)

// Protótipos de funções
void clear_screen(void);
void update_cursor(void);
void putchar(char c);
void print(const char *str);
void execute_command(void);
void handle_keypress(void);
void scroll_screen(void); // Adicionado protótipo
static inline void outb(unsigned short port, unsigned char value);
static inline unsigned char inb(unsigned short port);
static int strcmp(const char *s1, const char *s2);

// Variáveis globais
static int cursor_x = 0;
static int cursor_y = 0;
static char cmd_buffer[MAX_CMD_SIZE];
static int cmd_pos = 0;

// Funções de porta I/O
static inline void outb(unsigned short port, unsigned char value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Funções auxiliares
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Funções do terminal
void scroll_screen() {
    volatile char *video = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
        video[i] = video[i + VGA_WIDTH * 2];
    }
    // Limpar última linha
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i < VGA_HEIGHT * VGA_WIDTH * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = MAKE_COLOR(LIGHT_GREY, BLACK);
    }
}

void clear_screen() {
    volatile char *video = (volatile char*)VIDEO_MEMORY;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        video[i] = ' ';
        video[i + 1] = MAKE_COLOR(LIGHT_GREY, BLACK);
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

void putchar(char c) {
    volatile char *video = (volatile char*)VIDEO_MEMORY;
    int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;

    switch(c) {
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
        case '\r':
            cursor_x = 0;
            break;
        case '\b':
            if (cursor_x > 0) {
                cursor_x--;
                offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
                video[offset] = ' ';
                video[offset + 1] = MAKE_COLOR(LIGHT_GREY, BLACK);
            }
            break;
        default:
            video[offset] = c;
            video[offset + 1] = MAKE_COLOR(LIGHT_GREY, BLACK);
            cursor_x++;
            break;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VGA_HEIGHT) {
        scroll_screen();
        cursor_y = VGA_HEIGHT - 1;
    }
    update_cursor();
}

void print(const char *str) {
    while (*str) {
        putchar(*str++);
    }
}

void execute_command() {
    cmd_buffer[cmd_pos] = '\0';

    if (cmd_pos > 0) {
        if (strcmp(cmd_buffer, "clear") == 0) {
            clear_screen();
        } else {
            print("\nComando desconhecido: ");
            print(cmd_buffer);
        }
    }

    print("\n$ ");
    cmd_pos = 0;
}

// Mapa do teclado (US)
static const char kbd_us[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

void handle_keypress() {
    unsigned char status = inb(0x64);
    if (status & 0x01) {
        unsigned char scancode = inb(0x60);

        if (scancode & 0x80) {
            return; // Tecla solta
        }

        char c = kbd_us[scancode];
        if (c) {
            if (c == '\n') {
                putchar(c);
                execute_command();
            }
            else if (c == '\b') {
                if (cmd_pos > 0) {
                    cmd_pos--;
                    putchar(c);
                }
            }
            else if (cmd_pos < MAX_CMD_SIZE - 1) {
                cmd_buffer[cmd_pos++] = c;
                putchar(c);
            }
        }
    }
}

// Função principal
void kernel_main() {
    volatile char *video = (volatile char*)VIDEO_MEMORY;
    video[0] = 'K';
    video[1] = MAKE_COLOR(WHITE, BLACK);

    clear_screen();
    print("Kernel iniciado com sucesso!\n");
    print("Sistema Operacional v0.2\n");
    print("Comandos disponiveis:\n");
    print("  clear - Limpa a tela\n");
    print("\n$ ");

    while (1) {
        handle_keypress();
    }
}
