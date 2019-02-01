#include <highsyscall.h>
#include <uARMtypes.h>
#include <uARMconst.h>
#include <arch.h>
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
	int COMMAND=0|(sectNo<<8)|DEV_DISK_C_READBLK;
	int *DATA0=blockAddr;
	int bitmap=DEV_OVERWRITE_COMMAND|DEV_OVERWRITE_DATA0;
	setDeviceRegister(INT_DISK,diskNo,0,COMMAND,(unsigned int) DATA0, 0,bitmap);
}
void diskGet(int *blockAddr, int diskNo, int sectNo){

}
void writePrinter(char *virtAddr, int len){

}
void terminate(){

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
void getDeviceData1(int IntlineNo , int DevNo, int* DATA1){
	unsigned int *device;
	getDeviceRegister(IntlineNo, DevNo, &device);
	*DATA1=device[2];
}
void getDeviceRegister(int IntlineNo, int DevNo,unsigned int** device){
	*device=(int*)DEV_REG_START+((IntlineNo - 3) * DEV_REGBLOCK_SIZE) + (DevNo * DEV_REG_SIZE);
}
