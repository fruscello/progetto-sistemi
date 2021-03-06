
#include <highhandler.h>
#include <utility.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <init3.h>
#include <highsyscall.h>


void schedule(state_t *old){
	//tprint("in schedule\n");
	//if(old->a1==1) tprint("old_sys == 1\n");
	//updateTimer();
	//freezeLastTime(runningPcb); /* Freezing the lasttime in pcb for calculating next user time */
	LDST(old);
}
void highSysHandler(){
	//tprint("in highSysHandler!\n");
	state_t old_sys;
	old_sys=new_old_state_t[2];
	/*old_sys.a1=new_old_state_t[2].a1;
	old_sys.a2=new_old_state_t[2].a2;
	old_sys.a3=new_old_state_t[2].a3;
	old_sys.a4=new_old_state_t[2].a4;
	old_sys.v1=new_old_state_t[2].v1;
	old_sys.v2=new_old_state_t[2].v2;
	old_sys.v3=new_old_state_t[2].v3;
	old_sys.v4=new_old_state_t[2].v4;
	old_sys.v5=new_old_state_t[2].v5;
	old_sys.v6=new_old_state_t[2].v6;
	old_sys.sl=new_old_state_t[2].sl;
	old_sys.fp=new_old_state_t[2].fp;
	old_sys.ip=new_old_state_t[2].ip;
	old_sys.sp=new_old_state_t[2].sp;
	old_sys.lr=new_old_state_t[2].lr;
	old_sys.pc=new_old_state_t[2].pc;
	old_sys.cpsr=new_old_state_t[2].cpsr;
	old_sys.CP15_Control=new_old_state_t[2].CP15_Control;
	old_sys.CP15_EntryHi=new_old_state_t[2].CP15_EntryHi;
	old_sys.CP15_Cause=new_old_state_t[2].CP15_Cause;
	old_sys.TOD_Hi=new_old_state_t[2].TOD_Hi;
	old_sys.TOD_Low=new_old_state_t[2].TOD_Low;
	int k1,k2;*/
	
	//k1=0;
	//k2=1;
	switch(old_sys.a1){
		case READTERMINAL:
			tprint("READTERMINAL\n");
			readTerminal((char*) old_sys.a2);
			tprint("END READTERMINAL\n");
			break;
		case WRITETERMINAL:
			tprint("WRITETERMINAL\n");
			writeTerminal((char*)old_sys.a2,old_sys.a3);
			//tprint("END WRITETERMINAL\n");
			break;
		case VSEMVIRT:
			tprint("VSEMVIRT\n");
			break;
		case PSEMVIRT:
			tprint("PSEMVIRT\n");
			break;
		case DELAY:
			tprint("DELAY\n");
			break;
		case DISKPUT:
			tprint("DISK_PUT\n");
			//diskPut((int *)old_sys.a2,old_sys.a3,old_sys.a4);
			tprint("fine DISK_PUT\n");
			break;
		case DISKGET:
			tprint("DISK_GET\n");
			break;
		case WRITEPRINTER:
			tprint("WRITEPRINTER\n");
			writePrinter((char*) old_sys.a2, old_sys.a3);
			tprint("END PRINTER\n");
			break;
		case GETTOD:
			tprint("GETTOD\n");
			break;
		case TERMINATE:
			tprint("TERMINATE\n");
			break;
	}
	
	schedule(&old_sys);
	//schedule(&new_old_state_t[2]);
	//WAIT();
	tprint("scheduler finito\n");
}
