#include <highsyscall.h>
#include <highhandler.h>
#include <VMhandler.h>
#include <uARMtypes.h>
#include <uARMconst.h>
#include <libuarm.h>
#include <pcb.h>
#include <list.h>
#include <semaphore.h>
#include <arch.h>
#include <types.h>
#include <syscall.h>
#include <scheduler.h>
#include <main.h>
#include <init3.h>
#include <utility.h>

void readTerminal(char *virtAddr){

}
void writeTerminal(char *virtAddr, int len){

}
void virtualV(int *semaddr, int weight){

}
void virtualP(int *semaddr, int weight){

}
void delay(int secCnt){
	delayBlock(secCnt,&new_old_state_t[2]);
}
void diskPut(int *blockAddr, int diskNo, int sectNo){
	diskReadWrite(blockAddr, diskNo, sectNo, DEV_DISK_C_WRITEBLK);
}
void diskGet(int *blockAddr, int diskNo, int sectNo){
	diskReadWrite(blockAddr, diskNo, sectNo, DEV_DISK_C_READBLK);
}
//il readwirte e' caricato con il comando relativo a read/write (senza specificare head num e sect num)
void diskReadWrite(int *blockAddr, int diskNo, int sectNo,int readwirte){		
	/*int cyl_num = sectNo / MAXHEAD[diskNo] * MAXSECT[diskNo];
	int head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	int sect_num = sectNo-cyl_num-head_num;		//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
	*/
	int cyl_num,head_num,sect_num;
	trasformSectNo(diskNo, &cyl_num, &head_num, &sect_num, sectNo);
	disk_addr[diskNo]=(memaddr)blockAddr;
	//disk_addr[diskNo]=0x00010000;
	//getDeviceData0(INT_DISK, diskNo, &data0);
	//tprint("in diskPut\n");
			
	
	is_seeking_cyl[diskNo]=1;
	has_finished[diskNo]=0;
	
	
	
	
	//setto l'operazione da fare quando si sara' posizionato sul cilindro giusto
	device_operation[diskNo][DISK_MUTEX]=(((head_num<<8)|sect_num)<<8)|readwirte;//ATTENZIONE prima di togliere questa parte assicurati che con il disco 0 funzioni
	
	//riempo il buffer del disco in modo da tener traccia della prossima operazione da fare
	if((disk_buffer[diskNo].dim<MAXUPROC)&(diskNo!=0)){
		int first_emplty_space=(disk_buffer[diskNo].next_op+disk_buffer[diskNo].dim)%MAXUPROC;
		disk_buffer[diskNo].syl_op[first_emplty_space]=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		disk_buffer[diskNo].COMMAND[first_emplty_space]=(((head_num<<8)|sect_num)<<8)|readwirte;
		disk_buffer[diskNo].DATA0[first_emplty_space]=(memaddr)blockAddr;
		disk_buffer[diskNo].dim++;
		
	}
	if((diskNo==0)||(disk_buffer[diskNo].dim==1)){
		int bitmap=DEV_OVERWRITE_COMMAND;
		int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);			//posizionati sul ciclindro giusto!!!
		uproc[diskNo]=runningPcb;
	}
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	tprint("repeat debug: 3\n");
	if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	//if(diskNo!=0)			//se e' presente un tlb exception il processo e' gia stato bloccato, non serve bloccarlo di nuovo
		P(&device_mutex[diskNo][DISK_MUTEX]);
	//else tprint("diskNo!=0\n");
	tprint("repeat debug: 4\n");
	if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	
	//tprint("fine diskPut\n");
}
void writePrinter(char *virtAddr, int len){

}
void terminate(){

}
//e' un processo continuamente attivo
void delayDemon(){
	while(1){		//da fare in modo che quando rimane l'ultimo processo termina
		SYSCALL(WAITCLOCK,0,0,0);
		int i;
		for (i=0;i<MAXUPROC;i++){
			if(delay_need2unblock(i))
				delayUnblock(i);
			
		}
	}
}
int delay_need2unblock(int i){
	//da migliorare: c'e' da considerare il caso in cui vada usato Lo e bisogna scalare da cicli macchina a secondi
	if(delay_table[i].is_blocked=0) return 0;
	int time_hi=getTODHI();
	int time_lo=getTODLO();
	int time_waited_hi = time_lo-delay_table[i].startHi;
	int time_waited_lo = time_lo-delay_table[i].startLo;
	if(time_waited_lo>=delay_table[i].time2wait)
		return 1;
	else return 0;
	
	
}
void delayBlock(int secCnt, state_t *state){
	int to_block=getFirstDelayTableFree();
	delay_table[to_block].startHi=getTODHI();
	delay_table[to_block].startLo=getTODLO();
	delay_table[to_block].time2wait=secCnt;
	delay_table[to_block].is_blocked=1;
	delay_table[to_block].blocked_p_s=*state;
	softBlock(runningPcb);
	SYSCALL(SEMP,(int)&delay_table[to_block].sem,0,0);
	
	
	
	
}
void delayUnblock(int to_unblock){
	pcb_t *p=headBlocked(&(delay_table[to_unblock].sem));
	unsoftblock(p);
	p->p_s=delay_table[to_unblock].blocked_p_s;		//faccio in modo che torni allo stato in cui era prima di bloccarsi
	delay_table[to_unblock].is_blocked=0;
	SYSCALL(SEMV,(int)&delay_table[to_unblock].sem,0,0);
}
int getFirstDelayTableFree(){
	int i=0;
	while((i<MAXUPROC)&&(delay_table[i].is_blocked)){
		i++;
	}
	if(i<MAXUPROC)
		return i;
	else {tprint ("errore, delay_table pieno!!\n");return -1;}
}
void initDelay(){
	int i;
	for(i=0;i<MAXUPROC;i++){
		delay_table[i].sem=0;
		//delay_table[i].startHi=0;
		//delay_table[i].startLo=0;
		//delay_table[i].time2wait=0;
		delay_table[i].is_blocked=0;
	}
	initDiskBuffer();
}
void initDevices(){
	int i,j;
	for(j=0;j<DEV_MUTEX_LEN;j++){
		for(i=0;i<DEV_NUM;i++){
			device_mutex[i][j]=0;
			device_operation[i][j]=0;
		}
	}
	for(i=0;i<DEV_NUM;i++){
		disk_op_rdy[i]=0;
		is_seeking_cyl[i]=0;
		uproc[i]=0;
	}
	initDisk();
}
void initDiskBuffer(){
	int i;
	for(i=0;i<DEV_NUM;i++){
		int j;
		for(j=0;j<DEV_NUM;j++){
			disk_buffer[i].syl_op[j]=0;
			disk_buffer[i].COMMAND[j]=0;
			disk_buffer[i].DATA0[j]=0;
		}
		disk_buffer[i].next_op=0;
		disk_buffer[i].dim=0;
	}
}
void initDisk(){
	int i;
	int DATA1;
	for(i=0;i<DEV_NUM;i++){
		getDeviceData1(INT_DISK ,i, &DATA1);
		MAXSECT[i]=DATA1/0x80;
		MAXHEAD[i]=(DATA1>>8)/0x80;
		MAXCYL[i]=DATA1>>16;
	}
	initDiskBuffer();
}
void setDeviceRegister(int IntlineNo , int DevNo,unsigned int STATUS,unsigned int COMMAND,unsigned int DATA0,unsigned int DATA1,int bitmap){
	unsigned int *device;
	getDeviceRegisterHig(IntlineNo, DevNo, &device);
	if(bitmap&DEV_OVERWRITE_STATUS)
		device[0]=STATUS;
	if(bitmap&DEV_OVERWRITE_DATA0)
		device[2]=DATA0;
	if(bitmap&DEV_OVERWRITE_DATA1)
		device[3]=DATA1;
	//va messo per ultimo poiche appena lo metti parte
	if(bitmap&DEV_OVERWRITE_COMMAND)
		device[1]=COMMAND;
}
void getDeviceStatus(int IntlineNo , int DevNo, int* STATUS){
	unsigned int *device;
	getDeviceRegisterHig(IntlineNo, DevNo, &device);
	*STATUS=device[0];
}
void getDeviceData0(int IntlineNo , int DevNo, int* DATA0){
	unsigned int *device;
	getDeviceRegisterHig(IntlineNo, DevNo, &device);
	*DATA0=device[2];
}
void getDeviceData1(int IntlineNo , int DevNo, int* DATA1){
	unsigned int *device;
	getDeviceRegisterHig(IntlineNo, DevNo, &device);
	*DATA1=device[3];
}
void getDeviceRegisterHig(int IntlineNo, int DevNo,unsigned int** device){
	*device=(int*)(DEV_REG_START+((IntlineNo - 3) * DEV_REGBLOCK_SIZE) + (DevNo * DEV_REG_SIZE));
	a_sys_debug[0]=(int)*device;
	a_sys_debug[1]=DevNo;
}

void diskNextStep(int diskNo){
	int STATUS;
	getDeviceStatus(INT_DISK , diskNo, &STATUS);
	tprint("in diskNextStep\n");
	//pippo();
	if(diskNo==0)tprint("diskNo==0(diskNextStep)\n");
	if(STATUS!=1){
		a_sys_debug[2]=diskNo;
		tprint("disk error!\n");
		PANIC();
	}else if(!has_finished[diskNo]){
		int next_op=disk_buffer[diskNo].next_op;
		if(is_seeking_cyl[diskNo]){
			tprint("in diskNextStep (1)\n");
			is_seeking_cyl[diskNo]=0;
			int COMMAND=device_operation[diskNo][DISK_MUTEX];
			int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
			if(diskNo==0){
				setDeviceRegister(INT_DISK,diskNo,0,COMMAND,(unsigned int)disk_addr[diskNo],0,bitmap);		// esegui l'operazione richiesta!!!
			}else{
				setDeviceRegister(INT_DISK,diskNo,0,disk_buffer[diskNo].COMMAND[next_op],(unsigned int)disk_buffer[diskNo].DATA0[next_op],0,bitmap);
				
			}
		}else{
			tprint("repeat debug: 2\n");
			if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
			else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
			is_seeking_cyl[diskNo]=1;
			has_finished[diskNo]=1;
			tprint("in diskNextStep (2)\n");
			int bitmap=DEV_OVERWRITE_COMMAND;
			
			
			
			if(diskNo!=0){
				if(activePcbs==0)tprint("activePcbs==0 (1)\n");
				if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (1)\n");
				unsoftblock(uproc[diskNo]);
				if(activePcbs==0)tprint("activePcbs==0 (2)\n");
				if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (2)\n");
				//pcb_t *last_running_pcb=running_pcb;
				//state_t last_state=last_running_pcb->p_s;
				if(device_mutex[diskNo][DISK_MUTEX]==0) tprint("device_mutex[diskNo][DISK_MUTEX]==0\n");
				if(device_mutex[diskNo][DISK_MUTEX]>0) tprint("device_mutex[diskNo][DISK_MUTEX]>0\n");
				if(device_mutex[diskNo][DISK_MUTEX]<0) tprint("device_mutex[diskNo][DISK_MUTEX]<0\n");
				
				disk_buffer[diskNo].dim--;
				disk_buffer[diskNo].next_op=(disk_buffer[diskNo].next_op+1)%MAXUPROC;
				if(disk_buffer[diskNo].dim<=0){
					setDeviceRegister(INT_DISK,diskNo,0,DEV_C_ACK,0,0,bitmap);	
				}else{
					setDeviceRegister(INT_DISK,diskNo,0,disk_buffer[diskNo].syl_op[next_op],0,0,bitmap);
					
				}
				
				tprint("sblocco il processo (disk next step)\n");
				V(&device_mutex[diskNo][DISK_MUTEX],&(uproc[diskNo]->p_s));
				//SYSCALL(SEMV, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0,0);
				//tprint("pippo\n");
				//last_running_pcb->p_s=last_state;
				schedule((state_t*)INT_OLDAREA);
			}else{
				setDeviceRegister(INT_DISK,diskNo,0,DEV_C_ACK,0,0,bitmap);
				tlbNextStep();
			}
		}
	}else{
		tprint("interrupt finished\n");
	}
}
void softBlock(pcb_t *pcb){
	pcb->waitingOnIO=1;
	activePcbs--;
	softBlockedPcbs++;
}
void unsoftblock(pcb_t *p){
	p->waitingOnIO = 0;
	softBlockedPcbs--;
	activePcbs++;
}
void trasformSectNo(int diskNo, int *cyl_num, int *head_num, int *sect_num, int sectNo){
	*cyl_num = sectNo / MAXHEAD[diskNo] * MAXSECT[diskNo];
	*head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	*sect_num = sectNo-*cyl_num-*head_num;		//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
}
