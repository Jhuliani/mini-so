/* kernel/kernel.h */
#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>

// Memória de vídeo
#define VIDEO_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_CMD_SIZE 256

// Cores VGA
#define VGA_BLACK 0
#define VGA_BLUE 1
#define VGA_GREEN 2
#define VGA_CYAN 3
#define VGA_RED 4
#define VGA_MAGENTA 5
#define VGA_BROWN 6
#define VGA_LIGHT_GREY 7
#define VGA_DARK_GREY 8
#define VGA_LIGHT_BLUE 9
#define VGA_LIGHT_GREEN 10
#define VGA_LIGHT_CYAN 11
#define VGA_LIGHT_RED 12
#define VGA_LIGHT_MAGENTA 13
#define VGA_LIGHT_BROWN 14
#define VGA_WHITE 15

// Protótipos de funções
void kernel_main(void);
void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);
void terminal_scroll(void);
void keyboard_handler(void);
static inline void outb(uint16_t port, uint8_t value);
static inline uint8_t inb(uint16_t port);
int strcmp(const char *s1, const char *s2);

#endif
