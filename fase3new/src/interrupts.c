#include <interrupts.h>
#include <arch.h>
#include <uARMconst.h>
#include <libuarm.h>
#include <scheduler.h>
#include <syscall.h>
#include <highsyscall.h>
#include <semaphore.h>
#include <list.h>
#include <pcb.h>
#include <main.h>

int normalDevices[N_INTERRUPT_LINES - 4][DEV_PER_INT];
int terminals[DEV_PER_INT][2];

unsigned int pseudoClockTicks;    /* Number of times the pseudoclock caused an interrupt */
unsigned int agingTicks;  /* Number of times the aging caused an interrupt */


void interruptHandler()
{
    /*
       Understand from which line the interrupt came from
       if line == IL_TIMER
           call the timer handler routine
       else
           find the appropriate device register
           find the appropriate device semaphore
           V on that semaphore
           Ack the interrupt
        Give control back to a process
     */
	//tprint("in interruptHandler");
    unsigned int cause = ((state_t *) INT_OLDAREA)->CP15_Cause; /* interrupt cause (CP15_Cause) */
    int dispatchFlag = 0; /* 1 if the interrupt cause was the timer and a dispatch is necessary, 0 otherwise */
    pcb_t *p = NULL;    /* Will hold the process that will be "charged" with the spent kernel time */
    int still_soft_blocked=0;
    int debug=0;
    userTimeAccounting(( (state_t *) INT_OLDAREA)->TOD_Hi, ( (state_t *) INT_OLDAREA)->TOD_Low); /* Now I account user time from the last moment I calculated it */

	a_int_debug[0]=cause;
    if (CAUSE_IP_GET(cause,INT_TIMER)){
			/*tprint("repeat debug: timer interrupt\n");
			if(headBlocked(&device_mutex[0][DISK_MUTEX])==NULL)tprint("device_mutex[deviceNo][DISK_MUTEX]=null\n");
			else	tprint("device_mutex[deviceNo][DISK_MUTEX]!=null\n");*/
        dispatchFlag = handleTimer();
	//tprint("\nint timer\n");
    }
    else
    {
        int i;  /* Contains the interrupt line */
        int j;  /* Contains the interrupt device */
        for (i = INT_DISK; i <= INT_TERMINAL; i++)
            if (CAUSE_IP_GET(cause,i))
            {
                int bitmap = *((memaddr*)CDEV_BITMAP_ADDR(i));
		a_int_debug[3]=bitmap;
		a_int_debug[4]=CDEV_BITMAP_ADDR(i);
                for (j = 0; j < BYTELEN; j++)
                    if (bitmap & (1 << j))
                        break;
                break;
            }
        devreg_t *deviceRegister = (devreg_t *)DEV_REG_ADDR(i,j);
	unsigned int * reg=(unsigned int*)(DEV_REG_ADDR(IL_TERMINAL, j));        
	int which = -1; /* If the interrupt was from a terminal, this discriminates between
                           transimmsion and receipt */
	a_int_debug[1]=i;
	a_int_debug[2]=j;
        switch(i)
        {
            /*
               Single semaphore devices are handled in a uniform way
             */
            case INT_DISK:
		tprint("in INT_DISK\n");
		still_soft_blocked=1;
		/*if(j==0) tprint("j==0\n");
		if(j==1) tprint("j==1\n");
		if(j==2) tprint("j==2\n");
		if(j==3) tprint("j==3\n");
		if(j==4) tprint("j==4\n");
		if(j==5) tprint("j==5\n");
		if(j==6) tprint("j==6\n");
		if(j==7) tprint("j==7\n");
		if(j==8) tprint("j==8\n");*/	
		diskNextStep(j);	
		//tprint("fine INT_DISK\n");
		debug=1;
		break;
            case INT_TAPE:
            case INT_UNUSED:    /* in uARMconst.h this is the definition of the Network interrupt line */
            case INT_PRINTER:
                p = headBlocked(&normalDevices[i-INT_LOWEST][j]);   /* INT_LOWEST is an offset of 3,
                                                                       to map each device >= 3 to
                                                                       the right semaphore */
                if (p == NULL)
                {
                    tprint("No device blocked on semaphore (MESSAGE BY KERNEL, NOT P2TEST)");
                    PANIC();
                }
                p->p_s.a1 = deviceRegister->dtp.status; /* Returning the status of the device */
                V(&normalDevices[i-INT_LOWEST][j],(state_t *)INT_OLDAREA);
                deviceRegister->dtp.command = DEV_C_ACK; /* acknowledging the interrupt */
                break;
            case INT_TERMINAL:

		
	/* I must determine which of the two subdevices generated the interrupt */
                if ((/*deviceRegister->term.transm_status*/reg[2] & DEV_TERM_STATUS) == DEV_TTRS_S_CHARTRSM) 
								// Successful transmission 
                    which = TRANSM;
                else if ((/*deviceRegister->term.recv_status*/ reg[0] & DEV_TERM_STATUS) == DEV_TRCV_S_CHARRECV){ 
										// Successful receipt 
                	  
			which = RECV;
		}
                else
                {
                    tprint("Failed both transmission and receipt (MESSAGE BY KERNEL, NOT P2TEST)");
                    PANIC();
                }

		p=headBlocked(&term_addr[j].mutex);

                if (p == NULL)
                {
			//tprint("QUQUI\n");
                    	//tprint("No device blocked on semaphore (MESSAGE BY KERNEL, NOT P2TEST)\n");
                    	//PANIC();
			//dispatch(NULL);
		    	still_soft_blocked=1;			//se uno fa tprint in questo modo non dovrebbe crashare
                }
		else if (which == TRANSM)// Returning the status of the device
		         p->p_s.a1 = reg[2];//deviceRegister->term.transm_status; 

		else if (which == RECV) // Returning the status of the device
		            p->p_s.a1 = reg[0];//deviceRegister->term.recv_status;

		char ch;
		if (which == TRANSM)
				reg[3]=DEV_C_ACK;// acknowledging the interrupt
		else if (which == RECV){

				ch=(char)(reg[0] >> 8); //lo setto qui perch{ l-ACK resetta il registro
				//tprint("SANTA LUCIA\n");
				reg[1]=DEV_C_ACK;// acknowledging the interrupt
				//while(1);
		}
		//*************************END OLD PHASE 2*********************
		
		if(which==TRANSM && term_addr[j].length>0){
			//tprint("VOLGLIO STAMPARE\n");
			term_addr[j].index++;
			if(term_addr[j].in_printing==1 && term_addr[j].index<=term_addr[j].length){
		   		 still_soft_blocked=1;
				 debug=1;
		  		 WTnext(j);
				
			}
		}
		else if(which==RECV){
			//while(1);
			//char ch=(char)(reg[0] >> 8);
			char* addr=(char*)(term_addr[j].addr);
			addr[term_addr[j].index++]=ch;

			if(ch=='\0' || ch=='\n' || ch=='\r'){
				still_soft_blocked=0;
				unsoftblock(uproc[j][TERMINAL]);
				//tprint("\nMADRE DI DIO\n");
				V(&term_addr[j].mutex,(state_t*)INT_OLDAREA);
			}
			else{
				still_soft_blocked=1;
				debug=1;
				RTnext(j);
			}				

		}

		break;
		
        }
	if(!still_soft_blocked){
		p->waitingOnIO = 0;
		softBlockedPcbs--;
		activePcbs++;
	}
    }
    if(!tprint)
	    /* Accounting kernel time for this process */
	    kernelTimeAccounting(((state_t *) INT_OLDAREA)->TOD_Hi, ((state_t *) INT_OLDAREA)->TOD_Low, p);

    /* Giving back control to processes */

    if (dispatchFlag){
	//tprint("\ndispatch flag\n");
	dispatch((state_t*)INT_OLDAREA);
    }else if (runningPcb != NULL){ // Some process was running when the interrupt occurred 
	/*if(debug)
	tprint("runningPcb!=null (dispatch)\n");*/
	restoreRunningProcess((state_t*)INT_OLDAREA);
    }else{
	/*if(debug)
	tprint("runningPcb==null (dispatch)\n");*/
	dispatch(NULL);
    }
}

int handleTimer()
{
    /* 
       get Time of the day and save it in a variable
        case lastTimer of
            PSEUDOCLOCK
                unblock all processes on pseudoclockSem
                updateTimer(TOD)
            TIMESLICE
                suspend the current process
                save its user, kernel and wall clock time in the PCB
                updateTimer(TOD)
                dispatch()
            AGING
                increase the priority of all processes in the ready queue by one
                updateTimer()
     */
    int ret = 0;
    switch (lastTimerCause)
    {
        case PSEUDOCLOCK:
		//tprint("\npseudoclock\n");
            pseudoClockTicks++;
            while (pseudoClockSem < 0)
                V(&pseudoClockSem,(state_t *)INT_OLDAREA);
            break;
        case TIMESLICE:
		//tprint("\ntime slice\n");
            if (readyPcbs > 0)
                ret = 1;
            break;
        case AGING:
            {
		//tprint("\naging\n");
                agingTicks++;
                if (readyPcbs > 0)
                {
                    forallProcQ(readyQueue,increasePriority,NULL);
                    pcb_t *p = headProcQ(readyQueue);
                    if (p->p_priority > runningPcb->p_priority)
                        insertInReady(runningPcb,(state_t*)INT_OLDAREA);
                }
            }
            break;
    }

    return ret;
}

void updateTimer()
{
    unsigned int scale = *((unsigned int *) BUS_REG_TIME_SCALE);    /* Needed to convert CPU cycle into
                                                                     micro seconds */
    int pseudoDeadline, agingDeadline;
    pseudoDeadline = (int)(clockStartLO + ((pseudoClockTicks + 1)*PSEUDOCLOCKPERIOD)*scale -
            getTODLO());
    /* Time remaining until the next pseudoClockTick, in numbers of CPU cycles */
    if (pseudoDeadline <= ((int)(TIMESLICEPERIOD*scale)))
    {
        lastTimerCause = PSEUDOCLOCK;   /* The next interrupt will be a pseudoclock interrupt */
        if (pseudoDeadline > 0) /*  If the deadline has been passed (<0) the timer is not set. This way
                                    the timer is not acknowledged and the interrupt handler is called
                                    again, handling the "missed" interrupt and increasing the number
                                    of ticks so far
                                 */
            setTIMER(pseudoDeadline);
    }
    else
    {
        agingDeadline = (int)(clockStartLO + ((agingTicks + 1)*AGINGPERIOD)*scale - getTODLO());
        if (agingDeadline <= ((int)(TIMESLICEPERIOD*scale)))
        {
            lastTimerCause = AGING;
            if (agingDeadline > 0)
                setTIMER(agingDeadline);
        }
        else
        {
            lastTimerCause = TIMESLICE;
            setTIMER(TIMESLICEPERIOD*scale);
        }
    }
}
