#include <main.h>
#include <init3.h>
#include <types.h>
#include <pcb.h>
#include <scheduler.h>
#include <syscall.h>
#include <highsyscall.h>
#include <highhandler.h>
#include <VMhandler.h>
#include <arch.h>
#include <uARMconst.h>
#include <interrupts.h>
#include <libuarm.h>
#include <asl.h>



extern void test();
void initNewOld(){
	initHandler((memaddr)&new_old_state_t[1],(memaddr)tlbHighHandler);
	initHandler((memaddr)&new_old_state_t[3],(memaddr)highSysHandler);		//sys handler
	initHandler((memaddr)&new_old_state_t[5],(memaddr)0/*traphandler*/);
	/*for(int i=1;i<=5;i=i+2){
		new_old_state_t[i].sp-=QPAGE;
	}*/
	
}
void p11(){
	tprint("p11\n");
	//int i=0;
	while(1/*i<10*/){
		//i++;
		tprint("p11 in while\n");
	}
	tprint("p11 finito\n");
}
void init3(){
	//while(1){}
	//a_pippo();
	initDevices();
	initNewOld();
	initTlbHandler();
	initDelay();
	//initSegT(5);
	int SEG2=0x40000000;
	int PAGE_SIZE=4096;
	int *buffer;
	int pippo=0x00030000;
	*(int *)pippo=7;
	int *pippo_t=&PAGE_SIZE+PAGE_SIZE;
	buffer = (int *)(SEG2 + (20 * PAGE_SIZE));
	if(activePcbs==0)
		tprint("init3(1):activePcbs = 0\n");
	SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	
	if(activePcbs==0)
		tprint("init3(2): activePcbs == 0\n");
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);
	
	if(activePcbs==0)
		tprint("init3(3): activePcbs == 0\n");
	SYSCALL(DISK_PUT, (int)pippo, 1, 0);
	*(int *)pippo=8;
	SYSCALL(DISK_GET,(int)pippo, 1, 0);
	if(*(int *)pippo==7)tprint("TEST COMPLETATO CON SUCCESSO\n");
	else if(*(int *)pippo==8)tprint("TEST FALLITO (pippo=8)\n");
	else tprint("TEST FALLITO (pippo!=7 && pippo!=8\n");
	
	/*state_t p1_state;
	STST(&p1_state);
	p1_state.pc=(memaddr)p11;
	p1_state.CP15_EntryHi=ENTRYHI_ASID_SET(p1_state.CP15_EntryHi,4);
	//p1_state.CP15_Control=CP15_CONTROL_NULL;
	p1_state.CP15_Control=CP15_ENABLE_VM(p1_state.CP15_Control);
	LDST(&p1_state);*/
	
	tprint("in init3\n");
	HALT();
}

void main() {
	init2((memaddr)init3);
	tprint("in main\n");
	HALT();
}
