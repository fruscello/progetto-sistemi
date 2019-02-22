
#ifndef HIGHHANDLER_H
#define HIGHHANDLER_H
#include <types.h>
#include <arch.h>

int pc;
void schedule(state_t *old);
void highSysHandler();

#endif // MAIN_H
