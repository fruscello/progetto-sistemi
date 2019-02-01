#include <main.h>
#include <init3.h>
#include <types.h>
#include <pcb.h>
#include <scheduler.h>
#include <syscall.h>
#include <highsyscall.h>
#include <arch.h>
#include <uARMconst.h>
#include <interrupts.h>
#include <libuarm.h>
#include <asl.h>


extern void test();

void init3(){
	initDisk();
	tprint("in init3\n");
	HALT();
}

void main() {
	init2((memaddr)init3);
	tprint("in main\n");
	HALT();
}
