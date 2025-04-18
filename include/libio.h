#ifndef _LIBIO_H
#define _LIBIO_H

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#include "isrs.h"

extern void outb(uint16_t port, uint8_t value);
extern uint8_t inb(uint16_t port);

extern void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
extern void disable_cursor();
extern uint8_t get_cursor_x();
extern uint8_t get_cursor_y();
extern uint16_t get_cursor_pos();
extern void update_cursor_pos(uint8_t x, uint8_t y);
extern void printf(const char* string, uint8_t color, ...);
extern void print(const char* string, uint8_t color);
extern void print_c(char c, uint8_t color);
extern void clear_screen();
extern void set_string(int x, int y, const char* string, uint8_t color);
extern void set_string_f(int x, int y, const char* string, uint8_t color, ...);
extern void set_char(int x, int y, char c, uint8_t color);

extern void keyboard_handler(regs* r);
extern void keyboard_install();

#endif
