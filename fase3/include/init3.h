
#ifndef INIT3_H
#define INIT3_H
#define QPAGE 1024

#include <types.h>
#include <arch.h>
#include <pcb.h>

state_t new_old_state_t[6];		//old tlb, new tlb, old sys, new sys, old pgtrap, new pgtrap
int a_init3_debug[10];
int debug;
void initNewOld();
void p11();
void init3();

#endif // MAIN_H
