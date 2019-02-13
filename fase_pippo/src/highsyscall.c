#include <highsyscall.h>
#include <uARMtypes.h>
#include <uARMconst.h>
#include <libuarm.h>
#include <pcb.h>
#include <arch.h>
#include <types.h>
#include <syscall.h>
#include <main.h>
#include <init3.h>

void readTerminal(char *virtAddr){
/*
	char* hwAddr=virtAddr;

	unsigned int status;

	int dev_num;
	if(!getDEV(TERMINAL,&dev_num)) return;

	while (*hwAddr != '\0') {
		unsigned int command = DEV_TRCV_C_RECVCHAR;
		status = SYSCALL(IODEVOP, command, DEV_REG_ADDR(IL_TERMINAL, dev_number) + WS,0);
		if ((status & DEV_TERM_STATUS) != DEV_TTRS_S_CHARTRECV)
        		{
				tprint("(status & DEV_TERM_STATUS) != DEV_TTRS_S_CHARRECV");
				PANIC();
        		}
		*hwAddr=(status >> BYTELEN);
		hwAddr++;
	};
	
*/

}

void writeTerminal(char *virtAddr, int length){
	int dev_num=1;
	unsigned int* reg;

	term_addr[dev_num].addr=(memaddr)virtAddr;
	term_addr[dev_num].length=length;
	term_addr[dev_num].in_printing=1;
	term_addr[dev_num].index=1;


	getDeviceRegister(INT_TERMINAL, dev_num,&reg);
	reg[3]=(*virtAddr << BYTELEN) | DEV_TTRS_C_TRSMCHAR;

	uproc[dev_num][TERMINAL]=runningPcb;

	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	SYSCALL(SEMP, (memaddr)&device_mutex[dev_num][TERMINAL], 0,0);		//la v va fatta nell'handler

}

void virtualV(int *semaddr, int weight){
	(*semaddr)-=weight-1;
	SYSCALL(SEMP,(int)semaddr,0,0);
}
void virtualP(int *semaddr, int weight){
	int i=0;
	for(i=0;i<weight;i++)
		SYSCALL(SEMV,(int)semaddr,0,0);
}
void delay(int secCnt){

}
void diskPut(int *blockAddr, int diskNo, int sectNo){
	int cyl_num = sectNo / MAXHEAD[diskNo] * MAXSECT[diskNo];
	int head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	int sect_num = sectNo-cyl_num-head_num;	
	//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
	if(blockAddr==0) 
		tprint("blockAddr==0\n");
	else	
		tprint("blockAddr!=0\n");

	disk_addr[diskNo]=(memaddr)blockAddr;
	
	is_seeking_cyl[diskNo]=1;
	int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
	int bitmap=DEV_OVERWRITE_COMMAND;
	setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);
	
	device_operation[diskNo][DISK]=(((head_num<<8)|sect_num)<<8)|DEV_DISK_C_WRITEBLK;
	
	uproc[diskNo][0]=runningPcb;
	
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	SYSCALL(SEMP, (memaddr)&device_mutex[diskNo][DISK], 0,0);		//la v va fatta nell'handler
	
	
		/*is_seeking_cyl[diskNo]=1;
		int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		int bitmap=DEV_OVERWRITE_COMMAND;
		setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);
		//fai una P
		SYSCALL(SEMP, (memaddr)&disk_op_rdy[diskNo], 0, 0);			//la v va fatta nell'handler
			COMMAND=(((head_num<<8)|sect_num)<<8)|DEV_DISK_C_READBLK;
			bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
			setDeviceRegister(INT_DISK,diskNo,0,COMMAND,(unsigned int) DATA0, 0,bitmap);*/
		
	//SYSCALL(SEMV, (memaddr)&device_mutex[diskNo][DISK], 0, 0);
	//fai un'altra P ma con un altro semaforo magari
	//tprint("fine diskPut\n");
}
void diskGet(int *blockAddr, int diskNo, int sectNo){

}
void writePrinter(char *virtAddr, int len){
	char* hwAddr=virtAddr;//indirizzo risolto?

	int dev_num;
	getDEV(PRINTER,&dev_num);
	dev_num=0; //per il momento lasciamolo cosi
	int i=0;
	int bitmap=0;
	for(i=0; i<len; i++){
		
	}
	/*

		bitmap=DEV_OVERWRITE_COMMAND | DEV_OVERWRITE_DATA0;
		setDeviceRegister(INT_PRINTER,dev_num,0,DEV_PRNT_C_PRINTCHR,(int) hwAddr[i],0,bitmap);
		device_operation[dev_num][PRINTER]=DEV_PRNT_C_PRINTCHR;

        	uproc[dev_num][PRINTER]=runningPcb;

        	//sistemo le variabili di fase 2
        	softBlock(runningPcb);

        	SYSCALL(SEMP, (memaddr)&device_mutex[dev_num][PRINTER], 0,0);  
	*/
}
void terminate(){

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
		for(j=0;j<DEV_USED_INTS;j++)
			uproc[i][j]=0;
	}
	initDisk();
	init_term_addr();
}

void init_term_addr(){
	int i=0;
	for(i=0;i<DEV_NUM;i++){
		term_addr[i].mutex=0;
		term_addr[i].in_printing=0;
		term_addr[i].index=0;
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
}
void setDeviceRegister(int IntlineNo , int DevNo,unsigned int STATUS,unsigned int COMMAND,unsigned int DATA0,unsigned int DATA1,int bitmap){
	unsigned int *device;
	getDeviceRegister(IntlineNo, DevNo, &device);
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
	getDeviceRegister(IntlineNo, DevNo, &device);
	*STATUS=device[0];
}
void getDeviceData0(int IntlineNo , int DevNo, int* DATA0){
	unsigned int *device;
	getDeviceRegister(IntlineNo, DevNo, &device);
	*DATA0=device[2];
}
void getDeviceData1(int IntlineNo , int DevNo, int* DATA1){
	unsigned int *device;
	getDeviceRegister(IntlineNo, DevNo, &device);
	*DATA1=device[3];
}
void getDeviceRegister(int IntlineNo, int DevNo,unsigned int** device){
	*device=(int*)DEV_REG_START+((IntlineNo - 3) * DEV_REGBLOCK_SIZE) + (DevNo * DEV_REG_SIZE);
}
void diskNextStep(int deviceNo){
	if(is_seeking_cyl[deviceNo]){
		tprint("in diskNextStep (1)\n");
		is_seeking_cyl[deviceNo]=0;
		int COMMAND=device_operation[deviceNo][DISK];
		int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
		setDeviceRegister(INT_DISK,deviceNo,0,COMMAND,(unsigned int)disk_addr[DEV_NUM],0,bitmap);
		//while(1){}
		//tprint("fine diskNextStep (1)\n");
	}else{
		//is_seeking_cyl[deviceNo]=1;
		tprint("in diskNextStep (2)\n");
		unsoftblock(uproc[deviceNo][0]);
		int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
		setDeviceRegister(INT_DISK,deviceNo,0,0,0,0,bitmap);
		SYSCALL(SEMV, (memaddr)&device_mutex[deviceNo][DISK], 0,0);
		
	}
}
void softBlock(pcb_t *pcb){
	runningPcb->waitingOnIO=1;
	activePcbs--;
	softBlockedPcbs++;
}
void unsoftblock(pcb_t *p){
	p->waitingOnIO = 0;
	softBlockedPcbs--;
	activePcbs++;
}

int getDEV(int class, int *dev_number){
	int i=0;
	int found=0;
	for(i=0;i<DEV_PER_INT && !found;i++)
		if(uproc[class][i]==runningPcb){
			found=1;
			*dev_number;		
		}
	return found;
}

void WTnext(int dev_num){
	int i=0;
	/*
for(i=0;i<term_addr[dev_num].index;i++)
		tprint("MAMMATA\n");
*/
	if(term_addr[dev_num].index < term_addr[dev_num].length){
		tprint("in writeTERNextStep (1)\n");
		if(term_addr[dev_num].index==term_addr[dev_num].length-1)
			term_addr[dev_num].in_printing=0;

		int* virtAddr=(int *)(term_addr[dev_num].index + term_addr[dev_num].addr);

		int bitmap=DEV_OVERWRITE_COMMAND;
		unsigned int* reg=NULL;

		getDeviceRegister(INT_TERMINAL, dev_num,&reg);
		reg[3]= (*virtAddr<< BYTELEN) | DEV_TTRS_C_TRSMCHAR;
		//SYSCALL(SEMP, (memaddr)&(term_addr[dev_num].mutex), 0,0);
	}
	else{
		tprint("in writeNextStep (2)\n");
	
		term_addr[dev_num].in_printing=0;
		term_addr[dev_num].index=0;
		unsoftblock(uproc[dev_num][TERMINAL]);
		//SYSCALL(SEMV, (memaddr)&(term_addr[dev_num].mutex), 0,0);
		SYSCALL(SEMV, (memaddr)&device_mutex[dev_num][TERMINAL], 0,0);	
	}

	//SYSCALL(SEMV, (memaddr)&(term_addr[dev_num].mutex), 0,0);

		SYSCALL(SEMV, (memaddr)&device_mutex[dev_num][TERMINAL], 0,0);


}

