[bits 32]
[extern idtp]

global idt_load

section .text:
idt_load:
	lidt [idtp]
	sti
	ret
