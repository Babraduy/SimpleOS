#include <stdint.h>
#include "libio.h"
#include "libstr.h"
#include "idt.h"
#include "isrs.h"
#include "irq.h"
#include "timer.h"
#include "heap.h"

extern void main()
{
	idt_install();
	isrs_install();
	irq_install();
	timer_install();
	keyboard_install();
	init_heap();

	enable_cursor(14, 15);

	while (1)
	{
		set_string_f(VGA_WIDTH - 12, 0, "X:%d ,Y:%d ", 0x4f, get_cursor_x(), get_cursor_y());
	}
}
