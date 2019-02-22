
#ifndef INIT3_H
#define INIT3_H
#define QPAGE 1024

#include <types.h>
#include <arch.h>
#include <pcb.h>

state_t new_old_state_t[6];		//old tlb, new tlb, old sys, new sys, old pgtrap, new pgtrap
int a_init3_debug[10];
int debug;
memaddr to_launch;
state_t state_to_launch;

void initNewOld();
void p11();
void p12();
void disk_test1();
void disk_test2();
void init3();
void init3secondPart();

#endif // MAIN_H
