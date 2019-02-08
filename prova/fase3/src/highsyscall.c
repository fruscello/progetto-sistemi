#include <highsyscall.h>
#include <uARMtypes.h>
#include <uARMconst.h>
#include <libuarm.h>
#include <pcb.h>
#include <arch.h>
#include <types.h>
#include <syscall.h>
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
void diskPut(int *blockAddr, int diskNo, int sectNo, state_t *pippo){
	int cyl_num = sectNo / MAXHEAD[diskNo] * MAXSECT[diskNo];
	int head_num = sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) / MAXSECT[diskNo];
	int sect_num = sectNo-cyl_num-head_num;		//oppure sectNo % (MAXHEAD[diskNo] * MAXSECT[diskNo]) % MAXSECT[diskNo];
	int *DATA0=blockAddr;
	//getDeviceData0(INT_DISK, diskNo, &data0);
	tprint("in diskPut\n");
	/*if(device_mutex[diskNo][DISK_MUTEX]<=0)				//ATTENZIONE: da togliere
		tprint("device_mutex e' <=0!!!\n");
	else*/
	//SYSCALL(SPECHDL, SPECTLB, (memaddr)&new_old_state_t[0], (memaddr)&new_old_state_t[1]);
	SYSCALL(SEMP, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0, 0);		//la v va fatta nell'handler
	if(
	pippo->a1==new_old_state_t[2].a1)tprint("1\n");if(
	pippo->a2==new_old_state_t[2].a2)tprint("2\n");if(
	pippo->a3==new_old_state_t[2].a3)tprint("3\n");if(
	pippo->a4==new_old_state_t[2].a4)tprint("4\n");if(
	pippo->v1==new_old_state_t[2].v1)tprint("5\n");if(
	pippo->v2==new_old_state_t[2].v2)tprint("6\n");if(
	pippo->v3==new_old_state_t[2].v3)tprint("7\n");if(
	pippo->v4==new_old_state_t[2].v4)tprint("8\n");if(
	pippo->v5==new_old_state_t[2].v5)tprint("9\n");if(
	pippo->v6==new_old_state_t[2].v6)tprint("10\n");if(
	pippo->sl==new_old_state_t[2].sl)tprint("11\n");if(
	pippo->fp==new_old_state_t[2].fp)tprint("12\n");if(
	pippo->ip==new_old_state_t[2].ip)tprint("13\n");if(
	pippo->sp==new_old_state_t[2].sp)tprint("14\n");if(
	pippo->lr==new_old_state_t[2].lr)tprint("15\n");if(
	pippo->pc==new_old_state_t[2].pc)tprint("16\n");if(
	pippo->cpsr==new_old_state_t[2].cpsr)tprint("17\n");if(
	pippo->CP15_Control==new_old_state_t[2].CP15_Control)tprint("18\n");if(
	pippo->CP15_EntryHi==new_old_state_t[2].CP15_EntryHi)tprint("19\n");if(
	pippo->CP15_Cause==new_old_state_t[2].CP15_Cause)tprint("20\n");if(
	pippo->TOD_Hi==new_old_state_t[2].TOD_Hi)tprint("21\n");if(
	pippo->TOD_Low==new_old_state_t[2].TOD_Low)tprint("22\n");
	
		/*is_seeking_cyl[diskNo]=1;
		int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
		int bitmap=DEV_OVERWRITE_COMMAND;
		setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);
		//fai una P
		if(disk_op_rdy[diskNo]<=0)				//ATTENZIONE: da togliere
			tprint("disk_op_rdy e' <=0!!!\n");
		else
		SYSCALL(SEMP, (memaddr)&disk_op_rdy[diskNo], 0, 0);			//la v va fatta nell'handler
			COMMAND=(((head_num<<8)|sect_num)<<8)|DEV_DISK_C_READBLK;
			bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
			setDeviceRegister(INT_DISK,diskNo,0,COMMAND,(unsigned int) DATA0, 0,bitmap);
		*/
	//SYSCALL(SEMV, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0, 0);
	//fai un'altra P ma con un altro semaforo magari
	tprint("fine diskPut\n");
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
			device_mutex[i][j]=1;
		}
	}
	for(i=0;i<DEV_NUM;i++){
		disk_op_rdy[i]=0;
		is_seeking_cyl[i]=0;
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
