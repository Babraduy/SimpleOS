#include <stdint.h>
#include "irq.h"
#include "isrs.h"
#include "libio.h"
#include "idt.h"

void* irq_routines[16] = {
	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};

void irq_install_handler(int irq, void (* handler)(regs *r))
{
	irq_routines[irq] = handler;
}

void irq_uninstall(int irq)
{
	irq_routines[irq] = 0;
}

void irq_remap(void)
{
	outb(0x20, 0x11);
	outb(0xa0, 0x11);
	outb(0x21, 0x20);
	outb(0xa1, 0x28);
	outb(0x21, 0x04);
	outb(0xa1, 0x02);
	outb(0x21, 0x01);
	outb(0xa1, 0x11);
	outb(0x21, 0x00);
	outb(0xa1, 0x00);
}

void irq_install()
{
	irq_remap();

	idt_set_gate(32, (uint32_t)_irq0,0x08,0x8e);
	idt_set_gate(33, (uint32_t)_irq1,0x08,0x8e);
	idt_set_gate(34, (uint32_t)_irq2,0x08,0x8e);
	idt_set_gate(35, (uint32_t)_irq3,0x08,0x8e);
	idt_set_gate(36, (uint32_t)_irq4,0x08,0x8e);
	idt_set_gate(37, (uint32_t)_irq5,0x08,0x8e);
	idt_set_gate(38, (uint32_t)_irq6,0x08,0x8e);
	idt_set_gate(39, (uint32_t)_irq7,0x08,0x8e);
	idt_set_gate(40, (uint32_t)_irq8,0x08,0x8e);
	idt_set_gate(41, (uint32_t)_irq9,0x08,0x8e);
	idt_set_gate(42, (uint32_t)_irq10,0x08,0x8e);
	idt_set_gate(43, (uint32_t)_irq11,0x08,0x8e);
	idt_set_gate(44, (uint32_t)_irq12,0x08,0x8e);
	idt_set_gate(45, (uint32_t)_irq13,0x08,0x8e);
	idt_set_gate(46, (uint32_t)_irq14,0x08,0x8e);
	idt_set_gate(47, (uint32_t)_irq15,0x08,0x8e);
}

void _irq_handler(regs *r)
{
	void (*handler)(regs *r);

	handler = irq_routines[r->int_no - 32];
	if (handler)
	{
		handler(r);
	}

	if (r->int_no >= 40)
	{
		outb(0xa0, 0x20);
	}

	outb(0x20, 0x20);
}
