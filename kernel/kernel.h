#ifndef KERNEL_H
#define KERNEL_H

// Definições básicas
#define VIDEO_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_INPUT 256

// Protótipos de funções
void clear_screen(void);
void print_string(const char *str);
void put_char(char c, char attr);
void handle_keyboard(void);
void update_cursor(void);
void scroll_screen(void);

// Funções inline
static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

#endif