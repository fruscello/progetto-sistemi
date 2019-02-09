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

}
void writeTerminal(char *virtAddr, int len){

}
void virtualV(int *semaddr, int weight){

}
void virtualP(int *semaddr, int weight){

}
void delay(int secCnt){

}
void diskPut(int *blockAddr, int diskNo, int sectNo){
	int cyl_num = sectNo / MAXHEAD[diskNo] * MAXSECT[diskNo];
	int head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	int sect_num = sectNo-cyl_num-head_num;		//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
	if(blockAddr==0) tprint("blockAddr==0\n");
	else	tprint("blockAddr!=0\n");
	disk_addr[diskNo]=(memaddr)blockAddr;
	//disk_addr[diskNo]=0x00010000;
	//getDeviceData0(INT_DISK, diskNo, &data0);
	//tprint("in diskPut\n");
	
	is_seeking_cyl[diskNo]=1;
	int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
	int bitmap=DEV_OVERWRITE_COMMAND;
	//int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
	setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);
	
	device_operation[diskNo][DISK_MUTEX]=(((head_num<<8)|sect_num)<<8)|DEV_DISK_C_READBLK;
	
	uproc[diskNo]=runningPcb;
	
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
	SYSCALL(SEMP, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0,0);		//la v va fatta nell'handler
	
	
		/*is_seeking_cyl[diskNo]=1;
		int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		int bitmap=DEV_OVERWRITE_COMMAND;
		setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);
		//fai una P
		SYSCALL(SEMP, (memaddr)&disk_op_rdy[diskNo], 0, 0);			//la v va fatta nell'handler
			COMMAND=(((head_num<<8)|sect_num)<<8)|DEV_DISK_C_READBLK;
			bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
			setDeviceRegister(INT_DISK,diskNo,0,COMMAND,(unsigned int) DATA0, 0,bitmap);*/
		
	//SYSCALL(SEMV, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0, 0);
	//fai un'altra P ma con un altro semaforo magari
	//tprint("fine diskPut\n");
}
void diskGet(int *blockAddr, int diskNo, int sectNo){

}
void writePrinter(char *virtAddr, int len){

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
		uproc[i]=0;
	}
	initDisk();
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
	if(bitmap&DEV_OVERWRITE_COMMAND)
		device[1]=COMMAND;
	if(bitmap&DEV_OVERWRITE_DATA0)
		device[2]=DATA0;
	if(bitmap&DEV_OVERWRITE_DATA1)
		device[3]=DATA1;
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
		int COMMAND=device_operation[deviceNo][DISK_MUTEX];
		int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
		setDeviceRegister(INT_DISK,deviceNo,0,COMMAND,(unsigned int)disk_addr[DEV_NUM],0,bitmap);
		while(1){
			//tprint("pippo\n");
		}
		//tprint("fine diskNextStep (1)\n");
	}else{
		//is_seeking_cyl[deviceNo]=0;
		tprint("in diskNextStep (2)\n");
		unsoftblock(uproc[deviceNo]);
		SYSCALL(SEMV, (memaddr)&device_mutex[deviceNo][DISK_MUTEX], 0,0);
		
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
