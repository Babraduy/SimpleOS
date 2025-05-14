#include <stdint.h>
#include <stddef.h>
#include "libio.h"
#include "libstr.h"
#include "idt.h"
#include "isrs.h"
#include "irq.h"
#include "timer.h"
#include "heap.h"
#include "disk_driver.h"
#include "fat12.h"

extern void main()
{
	idt_install();
	isrs_install();
	irq_install();
	timer_install();
	keyboard_install();
	init_heap();

	enable_cursor(14, 15);

	if (!fat_install())
	{
		char* ret = (char*) fat_read_file("testdir/testfile.txt");
		if (ret != NULL) kprint(ret, 0x0d);
	}

	heap_status();
	list_disks();

	while(1)
	{
		set_stringf(VGA_WIDTH - 20, 3, "X: %d ", 0x1e, get_cursor_x());
		set_stringf(VGA_WIDTH - 20, 4, "Y: %d ", 0x1e, get_cursor_y());
	}
}
