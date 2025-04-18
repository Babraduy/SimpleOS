#ifndef _TIMER_H
#define _TIMER_H

#include "isrs.h"

extern void timer_handler(regs *r);
extern void timer_install();
extern void timer_phase(int hz);
extern void wait(int ticks);

#endif
