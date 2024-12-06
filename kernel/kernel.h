#ifndef KERNEL_H
#define KERNEL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CMD_BUFFER_SIZE 256
#define VGA_MEMORY 0xB8000

// VGA colors
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
#define VGA_YELLOW 14
#define VGA_WHITE 15

// Function prototypes
void kernel_main(void);
void terminal_initialize(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_writestring(const char* str);
void terminal_backspace(void);
void update_statusbar(void);
char keyboard_getchar(void);

#endif