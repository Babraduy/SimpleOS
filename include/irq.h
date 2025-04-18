#ifndef _IRQ_H
#define _IRQ_H

#include "isrs.h"

extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();

extern void irq_install_handler(int irq, void (*handler)(regs *r));
extern void irq_uninstall(int irq);
extern void irq_remap(void);
extern void irq_install();
extern void _irq_handler(regs* r);

#endif
