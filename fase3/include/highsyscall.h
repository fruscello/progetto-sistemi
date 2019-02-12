#ifndef HIGHSYSCALL_H
#define HIGHSYSCALL_H

//utili per il bitmap usato in setDeviceRegister
#define DEV_OVERWRITE_STATUS 1			//0001
#define DEV_OVERWRITE_COMMAND 2			//0010
#define DEV_OVERWRITE_DATA0 4			//0100
#define DEV_OVERWRITE_DATA1 8			//1000
#define DEV_NUM 8

//per gestire i mutex dei device
#define DISK_MUTEX 0
#define TAPE_MUTEX 1
#define PRINTER_MUTEX 2
#define DEV_MUTEX_LEN 3

#include <uARMtypes.h>
#include <pcb.h>


//int device_mutex[8][devNo];
int MAXCYL[DEV_NUM];
int MAXHEAD[DEV_NUM];
int MAXSECT[DEV_NUM];
//serve ad ottere l'esclusiva di un processo sull i-esimo device di classe j
int device_mutex[DEV_NUM][DEV_MUTEX_LEN];	
//serve a memorizzare l'operazione che deve essere fatta da un processo sull i-esimo device di classe j
int device_operation[DEV_NUM][DEV_MUTEX_LEN];	
//serve ad aspettare che il device venga posizionato sul cilindro corretto e che quindi possa essere effettuata l'operazione richiesta
int disk_op_rdy[DEV_NUM];				
//true se sta cercando il cilindro, serve all'handler per sapere se continuare diskPut/diskGet lasciati a meta' o cominciare una nuova operazione
int is_seeking_cyl[DEV_NUM];

int has_finished[DEV_NUM];
//indica il puntatore all'indirizzo di memoria da scrivere/leggere	
memaddr disk_addr[DEV_NUM];	
//indica per ogni numero di device, il processo assegnato (null se non e' ancora stato assegnato)
pcb_t *uproc[DEV_NUM];				


void readTerminal(char *virtAddr);
void writeTerminal(char *virtAddr, int len);
void virtualV(int *semaddr, int weight);
void virtualP(int *semaddr, int weight);
void delay(int secCnt);
void diskPut(int *blockAddr, int diskNo, int sectNo);
void diskGet(int *blockAddr, int diskNo, int sectNo);
void diskReadWrite(int *blockAddr, int diskNo, int sectNo,int readwirte);
void writePrinter(char *virtAddr, int len);
void terminate();

void initDevices();
void initDisk();
void setDeviceRegister(int IntlineNo , int DevNo,unsigned int STATUS,unsigned int COMMAND,unsigned int DATA0,unsigned int DATA1,int bitmap);
void getDeviceStatus(int IntlineNo , int DevNo, int* STATUS);
void getDeviceData1(int IntlineNo , int DevNo, int* DATA1);
void getDeviceRegister(int IntlineNo, int DevNo,unsigned int** device);
void pippo();
void diskNextStep(int deviceNo);
void softBlock(pcb_t *pcb);
void unsoftblock(pcb_t *p);
void trasformSectNo(int diskNo, int *cyl_num, int *head_num, int *sect_num, int sectNo);
#endif
