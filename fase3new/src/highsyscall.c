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

int getDEV(int class){
	int i=0;
	int found=-1;
	for(i=0;i<DEV_PER_INT && found<0;i++)
		if(uproc[class][i]==runningPcb)
			found=i;	
	return found;
}

void readTerminal(char *virtAddr){

	int dev_num=7;

	unsigned int* reg;


	term_addr[dev_num].addr=virtAddr;
	term_addr[dev_num].length=0;
	term_addr[dev_num].in_printing=1;
	term_addr[dev_num].index=0;
	term_addr[dev_num].old_state=new_old_state_t[2];

	reg=(unsigned int*)(DEV_REG_ADDR(IL_TERMINAL, dev_num));


	reg[1]=DEV_TRCV_C_RECVCHAR;

	uproc[dev_num][TERMINAL]=runningPcb;

	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	

	SYSCALL(SEMP, (memaddr)&term_addr[dev_num].mutex, 0,0);
	tprint("FINITO\n");
	LDST(&term_addr[dev_num].old_state);
}

void RTnext(int dev_num){

	unsigned int* reg=(unsigned int*)(DEV_REG_ADDR(IL_TERMINAL, dev_num));
	reg[1]=DEV_TRCV_C_RECVCHAR;

}

void writeTerminal(char *virtAddr, int len){

	int dev_num=7;

	unsigned int* reg;

	if(virtAddr[len-1]=='\0')
		len--;

	term_addr[dev_num].addr=virtAddr;
	term_addr[dev_num].length=len;
	term_addr[dev_num].in_printing=1;
	term_addr[dev_num].index=0;
	term_addr[dev_num].old_state=new_old_state_t[2];

	reg=(unsigned int*)(DEV_REG_ADDR(IL_TERMINAL, dev_num));


	reg[3]=(virtAddr[0] << BYTELEN) | DEV_TTRS_C_TRSMCHAR;

	uproc[dev_num][TERMINAL]=runningPcb;

	
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	SYSCALL(SEMP, (memaddr)&term_addr[dev_num].mutex, 0,0);
	tprint("FINITO\n");
	LDST(&term_addr[dev_num].old_state);
}

void WTnext(int dev_num){
	char* ch=term_addr[dev_num].addr;
	int index=term_addr[dev_num].index;
	int length=term_addr[dev_num].length;
	unsigned int* reg=(unsigned int*)DEV_REG_ADDR(IL_TERMINAL, dev_num);
	if(index < length ){
		tprint("in writeTERNextStep (1)\n");
		reg[3]= (ch[index]<< BYTELEN) | DEV_TTRS_C_TRSMCHAR;
	}
	else{
		tprint("in writeNextStep (2)\n");
	
		term_addr[dev_num].in_printing=0;
		term_addr[dev_num].index=0;
		unsoftblock(uproc[dev_num][TERMINAL]);
		reg[3]=DEV_C_ACK;
		V(&term_addr[dev_num].mutex,(state_t*)INT_OLDAREA);	
	}
}

void virtualV(int *semaddr, int weight){
	int i=0;
	for(i=0;i<weight;i++)
		V(semaddr,(state_t*)INT_OLDAREA);
}
void virtualP(int *semaddr, int weight){
	(*semaddr)-=weight-1;
	SYSCALL(SEMP,(memaddr)semaddr,0,0);
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
	int cyl_num,head_num,sect_num;
	trasformSectNo(diskNo, &cyl_num, &head_num, &sect_num, sectNo);
	if((cyl_num>MAXCYL[diskNo])||(head_num>MAXHEAD[diskNo])||(sect_num>MAXSECT[diskNo])||((int)blockAddr>RAM_TOP)){
		tprint("ATTENZIONE overflow parametri in input\n");
		a_sys_debug[0]=cyl_num;
		a_sys_debug[1]=head_num;
		a_sys_debug[2]=sect_num;
		a_sys_debug[3]=(int)blockAddr;
		a_sys_debug[4]=MAXCYL[diskNo];
		a_sys_debug[5]=MAXHEAD[diskNo];
		a_sys_debug[6]=MAXSECT[diskNo];
		a_sys_debug[7]=RAM_TOP;
		a_sys_debug[8]=sectNo;
		PANIC();
	}
	disk_addr[diskNo]=(memaddr)blockAddr;
			
	
	is_seeking_cyl[diskNo]=1;
	has_finished[diskNo]=0;
	
	
	
	
	//setto l'operazione da fare quando si sara' posizionato sul cilindro giusto
	device_operation[diskNo][DISK_MUTEX]=(((head_num<<8)|sect_num)<<8)|readwirte;//ATTENZIONE prima di togliere questa parte assicurati che con il disco 0 funzioni
	
	//riempo il buffer del disco in modo da tener traccia della prossima operazione da fare
	if(disk_buffer[diskNo].dim<MAXUPROC){
		int next_emplty_space=(disk_buffer[diskNo].next_op+disk_buffer[diskNo].dim)%MAXUPROC;
		disk_buffer[diskNo].syl_op[next_emplty_space]=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		disk_buffer[diskNo].COMMAND[next_emplty_space]=(((head_num<<8)|sect_num)<<8)|readwirte;
		disk_buffer[diskNo].DATA0[next_emplty_space]=(memaddr)blockAddr;
		disk_buffer[diskNo].dim++;
		disk_buffer[diskNo].pcb[next_emplty_space]=runningPcb;		
		disk_buffer[diskNo].state[next_emplty_space]=new_old_state_t[2];
		
	}
	if((diskNo==0)||(disk_buffer[diskNo].dim==1)){
		//tprint("inserisco il primo comando\n");
		int bitmap=DEV_OVERWRITE_COMMAND;
		int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);			//posizionati sul ciclindro giusto!!!
		//uproc[diskNo]=runningPcb;
	}
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	//tprint("repeat debug: 3\n");
	//if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	//else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	//if(diskNo!=0)			//se e' presente un tlb exception il processo e' gia stato bloccato, non serve bloccarlo di nuovo
	
	SYSCALL(SEMP,(int)&device_mutex[diskNo][DISK_MUTEX],0,0);	//blocco il processo finche l'operazione sul disco non viene terminata
	//while(1){}
	schedule(&state_to_unblock);
	//P(&device_mutex[diskNo][DISK_MUTEX]);

	
	//tprint("repeat debug: 4\n");
	//if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	//else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	
	//tprint("fine diskPut\n");
}
void writePrinter(char *virtAddr, int len){

	int dev_num=7;
	unsigned int* reg=(unsigned int*)(DEV_REG_ADDR(IL_PRINTER, dev_num));

	ptr_addr[dev_num].length=len;
	ptr_addr[dev_num].addr=virtAddr;
	ptr_addr[dev_num].index=0;
	ptr_addr[dev_num].in_printing=1;
	ptr_addr[dev_num].old_state=new_old_state_t[2];

	reg[2]=virtAddr[0];
	reg[1]=DEV_PRNT_C_PRINTCHR;

	uproc[dev_num][PRINTER]=runningPcb;
	softBlock(runningPcb);

	SYSCALL(SEMP, (memaddr)&ptr_addr[dev_num].mutex, 0,0);
	LDST(&ptr_addr[dev_num].old_state);
}

void WPnext(int dev_num){
	char* ch=ptr_addr[dev_num].addr;
	int index=ptr_addr[dev_num].index;
	int length=ptr_addr[dev_num].length;
	unsigned int* reg=(unsigned int*)DEV_REG_ADDR(IL_PRINTER, dev_num);

	if(index < length ){
		tprint("in writePRNTNextStep (1)\n");

		reg[2]=*(ch+index);
		reg[1]=DEV_PRNT_C_PRINTCHR;

	}
	else{
		tprint("in writePRNTNextStep (2)\n");
	
		ptr_addr[dev_num].in_printing=0;
		ptr_addr[dev_num].index=0;
		unsoftblock(uproc[dev_num][PRINTER]);
		V(&ptr_addr[dev_num].mutex,(state_t*)INT_OLDAREA);	
	}
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
		//uproc[i]=0;
	}
	initDisk();
	init_base_dev();
}

void init_base_dev(){
	int i=0;
	for(i=0;i<DEV_NUM;i++){
		term_addr[i].mutex=0;
		term_addr[i].in_printing=0;
		term_addr[i].index=0;

		ptr_addr[i].mutex=0;
		ptr_addr[i].in_printing=0;
		ptr_addr[i].index=0;

	}
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
		a_sys_debug[3]=STATUS;
		tprint("disk error!\n");
		while(1){tprint("disk error!\n");}
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
			//tprint("repeat debug: 2\n");
			//if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
			//else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
			is_seeking_cyl[diskNo]=1;
			tprint("in diskNextStep (2)\n");
			int bitmap=DEV_OVERWRITE_COMMAND;
			
			
			
			if(/*diskNo!=0*/1){
				//if(activePcbs==0)tprint("activePcbs==0 (1)\n");
				//if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (1)\n");
				unsoftblock(disk_buffer[diskNo].pcb[next_op]);
				//if(activePcbs==0)tprint("activePcbs==0 (2)\n");
				//if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (2)\n");
				//pcb_t *last_running_pcb=running_pcb;
				//state_t last_state=last_running_pcb->p_s;
				/*if(device_mutex[diskNo][DISK_MUTEX]==0) tprint("device_mutex[diskNo][DISK_MUTEX]==0\n");
				if(device_mutex[diskNo][DISK_MUTEX]>0) tprint("device_mutex[diskNo][DISK_MUTEX]>0\n");
				if(device_mutex[diskNo][DISK_MUTEX]<0) tprint("device_mutex[diskNo][DISK_MUTEX]<0\n");*/
				
				disk_buffer[diskNo].dim--;
				state_to_unblock=disk_buffer[diskNo].state[next_op];
				disk_buffer[diskNo].next_op=(disk_buffer[diskNo].next_op+1)%MAXUPROC;
				next_op=disk_buffer[diskNo].next_op;
				//disk_buffer[diskNo].pcb[next_op]->p_s=disk_buffer[diskNo].state[next_op];
				if(disk_buffer[diskNo].dim<=0){
					has_finished[diskNo]=1;
					setDeviceRegister(INT_DISK,diskNo,0,DEV_C_ACK,0,0,bitmap);	
				}else{
					setDeviceRegister(INT_DISK,diskNo,0,disk_buffer[diskNo].syl_op[next_op],0,0,bitmap);
					
				}
				
				tprint("sblocco il processo (disk next step)\n");
				SYSCALL(SEMV,(int)&device_mutex[diskNo][DISK_MUTEX],0,0);
				//V(&device_mutex[diskNo][DISK_MUTEX],&(uproc[diskNo]->p_s));
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
	*cyl_num = sectNo / (MAXHEAD[diskNo] * MAXSECT[diskNo]);
	*head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	*sect_num = sectNo-*cyl_num-*head_num;		//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
}
