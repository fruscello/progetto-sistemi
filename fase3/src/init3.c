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
void launcherVM(){
	tprint("launcherVM\n");
	SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);
	state_t p1_state;
	STST(&p1_state);
	p1_state.pc=to_launch;
	//p1_state.CP15_EntryHi=ENTRYHI_ASID_SET(p1_state.CP15_EntryHi,6);
	p1_state.CP15_Control=CP15_ENABLE_VM(STATUS_USER_MODE);
	//while(1){tprint("launch\n");}
	LDST(&p1_state);
	
	tprint("launcherVM finito\n");
}
void p11(){
	tprint("p11\n");
	/*state_t statep1;
	pcb_t *p;
	STST(&statep1);
	statep1.pc=(memaddr)p12;
	statep1.CP15_EntryHi=ENTRYHI_ASID_SET(statep1.CP15_EntryHi,5);
	SYSCALL(CREATEPROCESS, (int)&statep1, 1, (int)&p);*/
	int i=0;
	while(i<10){
		i++;
		tprint("p11 in while\n");
	}
	while(1){
		int seg2=0x80000000;
		int *a=(int *)seg2+4*WS;
		*a=5;
		a_init3_debug[0]=activePcbs;
		a_init3_debug[1]=softBlockedPcbs;
		a_init3_debug[2]=tlb_mutex;
		//a_init3_debug[3]=device_mutex[0][DISK_MUTEX];
		//a_init3_debug[4]=(int)p->p_semKey;
		//a_init3_debug[5]=*(p->p_semKey);
		tprint("p11 nel secondo while\n");
	}
	tprint("p11 finito\n");
}
void p12(){
	/*SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);*/
	tprint("p12\n");
	int i=0;
	/*while(i<10){
		i++;
		tprint("p12 in while\n");
	}*/
	while(1){
		int seg2=0x80000000;
		int *a=(int *)seg2+4*WS;
		*a=5;
		tprint("p12 nel secondo while\n");
	}
	tprint("p12 finito\n");
}
void disk_test1(){
	SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);
	state_t statep1;
	pcb_t *p;
	STST(&statep1);
	statep1.pc=(memaddr)disk_test2;
	statep1.CP15_EntryHi=ENTRYHI_ASID_SET(statep1.CP15_EntryHi,5);
	SYSCALL(CREATEPROCESS, (int)&statep1, 0, (int)&p);
	
	
	
	int SEG2=0x40000000;
	int PAGE_SIZE=4096;
	int *buffer;
	int pippo=0x00030000;
	*(int *)pippo=7;
	int *pippo_t=&PAGE_SIZE+PAGE_SIZE;
	buffer = (int *)(SEG2 + (20 * PAGE_SIZE));
	int i=0;
	while(1){
		tprint("ATTENZIONE in disk_test 1 !!!\n");
		SYSCALL(DISK_PUT, (int)pippo, 1, 0);
		SYSCALL(DISK_GET,(int)pippo, 1, 0);
		i++;
	}
	while(1){}
}
void disk_test2(){
	SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	SYSCALL(SPECHDL, SPECSYSBP,(memaddr) &new_old_state_t[2], (memaddr)&new_old_state_t[3]);
	SYSCALL(SPECHDL, SPECPGMT, (memaddr)&new_old_state_t[4], (memaddr)&new_old_state_t[5]);
	int SEG2=0x40000000;
	int PAGE_SIZE=4096;
	int *buffer;
	int pippo=0x00030000;
	*(int *)pippo=7;
	int *pippo_t=&PAGE_SIZE+PAGE_SIZE;
	buffer = (int *)(SEG2 + (20 * PAGE_SIZE));
	while(1){
		tprint("ATTENZIONE in disk_test 2 !!!\n");
		SYSCALL(DISK_PUT, (int)pippo, 1, 0);
		SYSCALL(DISK_GET,(int)pippo, 1, 0);
	}
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
	
	/*if(activePcbs==0)
		tprint("init3(3): activePcbs == 0\n");
	SYSCALL(DISK_PUT, (int)pippo, 1, 0);
	*(int *)pippo=8;
	SYSCALL(DISK_GET,(int)pippo, 1, 0);
	if(*(int *)pippo==7)tprint("TEST COMPLETATO CON SUCCESSO\n");
	else if(*(int *)pippo==8)tprint("TEST FALLITO (pippo=8)\n");
	else tprint("TEST FALLITO (pippo!=7 && pippo!=8\n");*/
	
	
	state_t p1_state;
	STST(&p1_state);
	p1_state.pc=(memaddr)launcherVM;
	to_launch=(memaddr)p11;
	p1_state.CP15_EntryHi=ENTRYHI_ASID_SET(p1_state.CP15_EntryHi,5);
	//p1_state.CP15_Control=CP15_ENABLE_VM(STATUS_USER_MODE);
	//LDST(&p1_state);
	SYSCALL(CREATEPROCESS, (int)&p1_state, 0, 0);
	int i=0;
	//while(i<10){tprint("waiting to launche p12\n");i++;}
	//to_launch=(memaddr)p12;
	//SYSCALL(CREATEPROCESS, (int)&p1_state, 0, 0);

	/*state_t p1_state;
	STST(&p1_state);
	p1_state.pc=(memaddr)disk_test1;
	//p1_state.CP15_EntryHi=ENTRYHI_ASID_SET(statep1.CP15_EntryHi,5);
	SYSCALL(CREATEPROCESS, (int)&p1_state, 0, (memaddr)NULL);*/
	
	int sem=0;
	//while(1){}
	SYSCALL(SEMP,(int) &sem, 0,0);
	tprint("in init3\n");
	HALT();
}

void main() {
	init2((memaddr)init3);
	tprint("in main\n");
	HALT();
}
