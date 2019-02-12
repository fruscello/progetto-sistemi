#include <highsyscall.h>
#include <highhandler.h>
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
	
	int COMMAND=(cyl_num<<8)|DEV_DISK_C_SEEKCYL;
	int bitmap=DEV_OVERWRITE_COMMAND;
	//int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
	setDeviceRegister(INT_DISK,diskNo,0,COMMAND,0,0,bitmap);			//posizionati sul ciclindro giusto!!!
	//pippo();
	
	//setto l'operazione da fare quando si sara' posizionato sul cilindro giusto
	device_operation[diskNo][DISK_MUTEX]=(((head_num<<8)|sect_num)<<8)|readwirte;
	
	uproc[diskNo]=runningPcb;
	
	//sistemo le variabili di fase 2
	softBlock(runningPcb);
	
			tprint("repeat debug: 3\n");
	if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	P(&device_mutex[diskNo][DISK_MUTEX]);
			tprint("repeat debug: 4\n");
	if(headBlocked(&device_mutex[diskNo][DISK_MUTEX])==NULL)tprint("device_mutex[diskNo][DISK_MUTEX]=null\n");
	else	tprint("device_mutex[diskNo][DISK_MUTEX]!=null\n");
	//SYSCALL(SEMP, (memaddr)&device_mutex[diskNo][DISK_MUTEX], 0,0);		//la v va fatta nell'handler
	
	
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
void pippo(){}
void diskNextStep(int deviceNo){
	int STATUS;
	getDeviceStatus(INT_DISK , deviceNo, &STATUS);
	tprint("in diskNextStep\n");
	if(STATUS!=1){
		tprint("disk error!\n");
		PANIC();
	}else if(!has_finished[deviceNo]){
		if(is_seeking_cyl[deviceNo]){	
			tprint("in diskNextStep (1)\n");
			is_seeking_cyl[deviceNo]=0;
			int COMMAND=device_operation[deviceNo][DISK_MUTEX];
			int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
			setDeviceRegister(INT_DISK,deviceNo,0,COMMAND,(unsigned int)disk_addr[deviceNo],0,bitmap);		// esegui l'operazione richiesta!!!
			//pippo();
			/*int status=1;
			while(status){
				getDeviceStatus(INT_DISK, deviceNo, &status);
				if(status==1)
					status=0;
				tprint("repeat debug: 1\n");
				if(headBlocked(&device_mutex[deviceNo][DISK_MUTEX])==NULL)tprint("device_mutex[deviceNo][DISK_MUTEX]=null\n");
				else	tprint("device_mutex[deviceNo][DISK_MUTEX]!=null\n");
			}
			tprint("fine debug: 1\n");*/
			//while(1){}
			//tprint("fine diskNextStep (1)\n");
		}else{
			tprint("repeat debug: 2\n");
			if(headBlocked(&device_mutex[deviceNo][DISK_MUTEX])==NULL)tprint("device_mutex[deviceNo][DISK_MUTEX]=null\n");
			else	tprint("device_mutex[deviceNo][DISK_MUTEX]!=null\n");
			//is_seeking_cyl[deviceNo]=1;
			has_finished[deviceNo]=1;
			tprint("in diskNextStep (2)\n");
			int bitmap=DEV_OVERWRITE_COMMAND;
			setDeviceRegister(INT_DISK,deviceNo,0,DEV_C_ACK,0,0,bitmap);
			
			if(activePcbs==0)tprint("activePcbs==0 (1)\n");
			if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (1)\n");
			unsoftblock(uproc[deviceNo]);
			if(activePcbs==0)tprint("activePcbs==0 (2)\n");
			if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (2)\n");
			//pcb_t *last_running_pcb=running_pcb;
			//state_t last_state=last_running_pcb->p_s;
			if(device_mutex[deviceNo][DISK_MUTEX]==0) tprint("device_mutex[deviceNo][DISK_MUTEX]==0\n");
			if(device_mutex[deviceNo][DISK_MUTEX]>0) tprint("device_mutex[deviceNo][DISK_MUTEX]>0\n");
			if(device_mutex[deviceNo][DISK_MUTEX]<0) tprint("device_mutex[deviceNo][DISK_MUTEX]<0\n");
			V(&device_mutex[deviceNo][DISK_MUTEX],&(uproc[deviceNo]->p_s));
			//SYSCALL(SEMV, (memaddr)&device_mutex[deviceNo][DISK_MUTEX], 0,0);
			//tprint("pippo\n");
			//last_running_pcb->p_s=last_state;
			schedule((state_t*)INT_OLDAREA);
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
