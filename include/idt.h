#ifndef _IDT_H
#define _IDT_H

extern void idt_load();

typedef struct idt_entry
{
	uint16_t base_lo;
	uint16_t sel;
	uint8_t always0;
	uint8_t flags;
	uint16_t base_hi;
} __attribute__((packed)) idt_entry;

typedef struct idt_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) idt_ptr;

extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
extern void idt_install();

#endif
