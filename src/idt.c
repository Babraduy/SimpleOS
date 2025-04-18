#include <stdint.h>
#include "libstr.h"
#include "idt.h"

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags)
{
	idt[num].base_hi = base >> 16;
	idt[num].base_lo = base & 0xFFFF;

	idt[num].always0 = 0;
	idt[num].flags = flags;
	idt[num].sel = sel;
}

void idt_install()
{
	idtp.limit = (sizeof(idt_entry) * 256) - 1;
	idtp.base = (uint32_t)idt;

	memset(&idt, 0, sizeof(idt_entry) * 256);

	idt_load();
}
