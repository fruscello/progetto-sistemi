#include <init3.h>
#include <types.h>
#include <pcb.h>
#include <scheduler.h>
#include <syscall.h>
#include <arch.h>
#include <uARMconst.h>
#include <interrupts.h>
#include <libuarm.h>
#include <asl.h>


extern void test();

void init3() {
	tprint("init()\n");
	HALT();
}

