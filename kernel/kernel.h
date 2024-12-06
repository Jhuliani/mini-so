#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

// Funções de porta I/O
static inline void outb(uint16_t port, uint8_t value);
static inline void outw(uint16_t port, uint16_t value);
static inline uint8_t inb(uint16_t port);

// Funções de string
size_t strlen(const char* str);
int strncmp(const char* s1, const char* s2, size_t n);

// Funções de terminal
void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_writestring(const char* data);
void terminal_backspace(void);

// Função de teclado
char keyboard_getchar(void);

#endif