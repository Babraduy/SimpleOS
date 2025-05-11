#include <stdint.h>
#include <stdarg.h>
#include "libio.h"
#include "libstr.h"
#include "timer.h"
#include "isrs.h"
#include "irq.h"
#include "heap.h"

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
	outb(0x3d4, 0x0a);
	outb(0x3d5, (inb(0x3d5) & 0xc0) | cursor_start);
	
	outb(0x3d4, 0x0b);
	outb(0x3d5, (inb(0x3d5) & 0xe0) | cursor_end);
}

void disable_cursor()
{
	outb(0x3d4, 0x0a);
	outb(0x3d5, inb(0x3d5) & ~(1 << 5));
}

void update_cursor_pos(uint8_t x, uint8_t y)
{
	uint16_t pos = y*VGA_WIDTH + x;

	outb(0x3d4, 0x0e);
	outb(0x3d5, pos >> 8);
	
	outb(0x3d4, 0x0f);
	outb(0x3d5, pos & 0x00FF);
}

uint16_t get_cursor_pos()
{
	outb(0x3d4, 0x0f);
	uint8_t low = inb(0x3d5);

	outb(0x3d4, 0x0e);
	uint8_t high = inb(0x3d5);

	uint16_t pos = ((uint16_t)high << 8) | low;

	return pos;
}

uint8_t get_cursor_x()
{
	return get_cursor_pos() % VGA_WIDTH;
}

uint8_t get_cursor_y()
{
	return get_cursor_pos() / VGA_WIDTH;
}

void kprintf(const char* string, uint8_t color, ...)
{
	va_list args;
	va_start(args, color);

	while (*string)
	{
		if (*string == '%')
		{
			char temp[20] = "";
			temp[19] = '\0';
			switch(*(string + 1))
			{
				case 'd':
					int int_arg = va_arg(args, int);
					itoa(int_arg, temp, 10);
					kprint(temp, color);
					string += 2;
					continue;

				case 'l':
					long long_arg = va_arg(args, long);
					itoa(long_arg, temp, 10);
					kprint(temp, color);
					string += 2;
					continue;

				case 'x':
					int hex_arg = va_arg(args, int);
					itoa(hex_arg, temp, 16);
					kprint(temp, color);
					string += 2;
					continue;

				case 's':
					char* str_arg = va_arg(args, char*);
					kprint(str_arg, color);
					string += 2;
					continue;

				case 'u':
					uint32_t uint_arg = va_arg(args, uint32_t);
					utoa(uint_arg, temp, 10);
					kprint(temp, color);
					string += 2;
					continue;
			}
		}

		kprint_c(*string, color);
		string++;
	}

	va_end(args);
}

void kprint(const char* string, uint8_t color)
{
	for (int i = 0; i < strlen(string);i++)
	{
		kprint_c(string[i], color);
	}
}

void kprint_c(char c, uint8_t color)
{
	char* vga = (char*) 0xb8000;

	if (c == '\b')		// backspace
	{
		vga[(get_cursor_pos() - 1) * 2] = ' ';
		vga[(get_cursor_pos() - 1) * 2 + 1] = color;

		if (get_cursor_x() == 0) return;

		update_cursor_pos(get_cursor_x() - 1, get_cursor_y());

		return;
	}

	if (get_cursor_x() == VGA_WIDTH - 1 && get_cursor_y() == VGA_HEIGHT - 1) return;

	if (c == '\n')		// enter
	{
		update_cursor_pos(0, get_cursor_y() + 1);
		return;
	}

	vga[get_cursor_pos() * 2] = c;
	vga[get_cursor_pos() * 2 + 1] = color;

	update_cursor_pos(get_cursor_x() + 1, get_cursor_y());
}

void clear_screen()
{
	char* vga = (char*) 0xb8000;

	update_cursor_pos(0,0);

	for (int i=0;i<VGA_HEIGHT; i++)
	{
		vga[i*2] = ' ';
		vga[i*2 + 1] = 0x00;
	}
}

void set_string(int x, int y, const char* string, uint8_t color)
{
	for (int i=0; string[i] != '\0'; i++)
	{
		set_char(x, y, string[i], color);
		x++;
	}
}

void set_stringf(int x, int y, const char* string, uint8_t color, ...)
{
	va_list args;
	va_start(args, color);

	while (*string)
	{
		if (*string == '%')
		{
			char temp[20] = "";
			char* p;
			switch(*(string + 1))
			{
				case 'd':
					int int_arg = va_arg(args, int);
					itoa(int_arg, temp, 10);

					p = temp;
					while (*p)
					{
						set_char(x,y,*p,color);
						x++;
						p++;
					}

					string += 2;
					continue;

				case 'l':
					long long_arg = va_arg(args, long);
					itoa(long_arg, temp, 10);
					
					p = temp;
					while (*p)
					{
						set_char(x,y,*p,color);
						x++;
						p++;
					}

					string += 2;
					continue;

				case 'x':
					int hex_arg = va_arg(args, int);
					itoa(hex_arg, temp, 16);

					p = temp;
					while (*p)
					{
						set_char(x,y,*p,color);
						x++;
						p++;
					}

					string += 2;
					continue;

				case 's':
					char* str_arg = va_arg(args, char*);
					p = str_arg;

					while (*p)
					{
						set_char(x,y,*p,color);
						x++;
						p++;
					}

					string += 2;
					continue;

				case 'u':
					uint32_t uint_arg = va_arg(args, uint32_t);
					utoa(uint_arg, temp, 10);

					p = temp;
					while (*p)
					{
						set_char(x,y,*p, color);
						x++;
						p++;
					}

					string += 2;
					continue;
			}
		}

		set_char(x, y, *string, color);

		string++;
		x++;
	}

	va_end(args);
}

void set_char(int x, int y, char c, uint8_t color)
{
	char* vga = (char*) 0xb8000;

	int pos = y * VGA_WIDTH + x;

	vga[pos * 2] = c;
	vga[pos * 2 + 1] = color;
}


int keystatus = 0; // 0x0 = nothing, 0x01 = shift, 0x02 = ctrl, 0x04 = alt, 0x08 = caps lock, 0x10 = num lock, 0x20 = scroll lock

int extended = 0;

char keymap[128] = {
	0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
	'9', '0', '-', '=', '\b',	/* Backspace */
	0,			/* Tab */
	'q', 'w', 'e', 'r',	/* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
	'\'', '`',   0,		/* Left shift */
	0, 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
	'm', ',', '.', '/',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */	
};

char shift_keymap[128] = {
	0,  27, '!', '@', '#', '$', '%', '^', '&', '*',	/* 9 */
	'(', ')', '_', '+', '\b',	/* Backspace */
	0,			/* Tab */
	'Q', 'W', 'E', 'R',	/* 19 */
	'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',	/* Enter key */
	0,			/* 29   - Control */
	'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',	/* 39 */
	'"', '~',   0,		/* Left shift */
	0, 'Z', 'X', 'C', 'V', 'B', 'N',			/* 49 */
	'M', '<', '>', '?',   0,				/* Right shift */
	'*',
	0,	/* Alt */
	' ',	/* Space bar */
	0,	/* Caps lock */
	0,	/* 59 - F1 key ... > */
	0,   0,   0,   0,   0,   0,   0,   0,
	0,	/* < ... F10 */
	0,	/* 69 - Num lock*/
	0,	/* Scroll Lock */
	0,	/* Home key */
	0,	/* Up Arrow */
	0,	/* Page Up */
	'-',
	0,	/* Left Arrow */
	0,
	0,	/* Right Arrow */
	'+',
	0,	/* 79 - End key*/
	0,	/* Down Arrow */
	0,	/* Page Down */
	0,	/* Insert Key */
	0,	/* Delete Key */
	0,   0,   0,
	0,	/* F11 Key */
	0,	/* F12 Key */
	0,	/* All other keys are undefined */	
};

void keyboard_handler(regs* r)
{
	uint8_t scancode = inb(0x60);

	if (scancode == 0xe0)
	{
		extended = 1;
		return;
	}

	if (scancode & 0x80)	// key released
	{
		if (scancode == 0xaa || scancode == 0xb6) keystatus &= ~(0x01);			// shift
		if (scancode == 0x9d || (extended && scancode == 0x9d)) keystatus &= ~(0x02);	// control
		if (scancode == 0xb8 || (extended && scancode == 0xb8)) keystatus &= ~(0x04);	// alt
	}
	else			// key pressed
	{
		if (scancode == 0x2a || scancode == 0x36) keystatus |= 0x01;			// shift
		if (scancode == 0x1d || (extended && scancode == 0x1d)) keystatus |= 0x02;	// control
		if (scancode == 0x38 || (extended && scancode == 0x38)) keystatus |= 0x04;	// alt

		if (extended && scancode == 0x48)	// up arrow
		{
			if (get_cursor_y() > 0) update_cursor_pos(get_cursor_x(), get_cursor_y() - 1);
		}
		if (extended && scancode == 0x4b)	// left arrow
		{
			if (get_cursor_x() > 0) update_cursor_pos(get_cursor_x() - 1, get_cursor_y());
			else if (get_cursor_x() == 0 && get_cursor_y() > 0) update_cursor_pos(VGA_WIDTH-1, get_cursor_y() - 1);
		}
		if (extended && scancode == 0x4d)	// right arrow
		{
			if (get_cursor_x() < VGA_WIDTH - 1) update_cursor_pos(get_cursor_x() + 1, get_cursor_y());
			else if (get_cursor_x() == VGA_WIDTH - 1 && get_cursor_y() < VGA_HEIGHT - 1) update_cursor_pos(0, get_cursor_y() + 1);
		}
		if (extended && scancode == 0x50)	// down arrow
		{
			if (get_cursor_y() < VGA_HEIGHT - 1) update_cursor_pos(get_cursor_x(), get_cursor_y() + 1);
		}

		if (keymap[scancode] != 0)		// any other key
		{
			if ((keystatus & 0x01) == 0) kprint_c(keymap[scancode], 0x0d);
			else kprint_c(shift_keymap[scancode], 0x0d);
		}
	}

	extended = 0;
}

void keyboard_install()
{
	irq_install_handler(1, keyboard_handler);
}
