#include <main.h>
#include <init3.h>
#include <types.h>
#include <pcb.h>
#include <scheduler.h>
#include <syscall.h>
#include <highsyscall.h>
#include <highhandler.h>
#include <arch.h>
#include <uARMconst.h>
#include <interrupts.h>
#include <libuarm.h>
#include <asl.h>



extern void test();
void initNewOld(){
	initHandler((memaddr)&new_old_state_t[1],(memaddr)0/*tlbhandler*/);
	initHandler((memaddr)&new_old_state_t[3],(memaddr)highSysHandler);		//sys handler
	initHandler((memaddr)&new_old_state_t[5],(memaddr)0/*traphandler*/);
	
}
void init3(){
	initDevices();
	initNewOld();
	
	if(activePcbs==0)
		tprint("init3(1):activePcbs = 0\n");
	//SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	
	if(activePcbs==0)
		tprint("init3(2): activePcbs == 0\n");
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);
	
	if(activePcbs==0)
		tprint("init3(3): activePcbs == 0\n");
	SYSCALL(DISK_PUT, 0, 0, 0);
	tprint("in init3\n");
	HALT();
}

void main() {
	init2((memaddr)init3);
	tprint("in main\n");
	HALT();
}
