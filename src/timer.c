#include <stdint.h>
#include "timer.h"
#include "isrs.h"
#include "libio.h"
#include "irq.h"

int timer_ticks = 0;

void timer_phase(int hz)
{
	int divisor = 1193180 / hz;
	outb(0x43, 0x36);
	outb(0x40, divisor & 0xff);
	outb(0x40, divisor >> 8);
}

void timer_handler(regs *r)
{
	timer_ticks++;
}

void timer_install()
{
	timer_phase(100);

	irq_install_handler(0, timer_handler);
}

void wait(int ticks)
{
	int start_ticks = timer_ticks;
	while (timer_ticks - start_ticks < ticks);
}
