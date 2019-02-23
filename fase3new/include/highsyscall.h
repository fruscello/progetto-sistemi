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

#define DISK 0
#define TAPE 1
#define NETWORK 2
#define PRINTER 3
#define TERMINAL 4

#include <uARMtypes.h>
#include <pcb.h>
#include <VMhandler.h>


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
pcb_t *uproc[DEV_NUM][DEV_USED_INTS];

typedef struct {
	int syl_op[MAXUPROC];
	int COMMAND[MAXUPROC];
	memaddr DATA0[MAXUPROC];
	pcb_t *pcb[MAXUPROC];
	state_t state[MAXUPROC];
	int next_op;
	int dim;
} buffer;

typedef struct {
	int sem;	//semaforo su cui bloccare il processo. Un processo bloccato su ogni semaforo
	int startHi;		//momento in cui viene bloccato (da cui comincia ad aspettare)
	int startLo;
	int time2wait;		//tempo che il processo deve aspettare
	int is_blocked;		//true se il processo e' bloccato
	state_t blocked_p_s;
} delay_blocked;	

typedef struct dev_param{
	char* addr;
	//int mode;//0=recv 1=trans
	int length;
	int index;
	int in_printing;
	int mutex;
	state_t old_state;
}dev_param;

typedef struct {
	int *vir_sem;		//semaforo virtuale su cui i processi devono essere indirettamente bloccati
	int sem;		//semaforo su cui bloccare i processi bloccati
	int blocked_proc_num;
	
}sem_struct;

//per il disco 0 non c'e' buffer, e' gestito separatamente, quindi ne basterebbero DEV_NUM-1 non servirebbe, ma lo tengo per semplicita'
buffer disk_buffer[DEV_NUM];

dev_param term_addr[DEV_NUM];
dev_param ptr_addr[DEV_NUM];

delay_blocked delay_table[MAXUPROC];
state_t state_to_unblock;
state_t sys_ret_state[MAXUPROC];		//ogni uproc ha il suo "indirizzo di ritorno", alla posizione del suo ASID

int a_sys_debug[10];


sem_struct sem_pv[MAXUPROC];		//struttura per la gestione delle syscall P e V

int getDEV(int class);
void readTerminal(char *virtAddr);
void RTnext(int dev_num);

void writeTerminal(char *virtAddr, int len);
void WTnext(int dev_num);

void virtualV(int *semaddr, int weight);
void virtualP(int *semaddr, int weight);
void delay(int secCnt);
void diskPut(int *blockAddr, int diskNo, int sectNo);
void diskGet(int *blockAddr, int diskNo, int sectNo);
void diskReadWrite(int *blockAddr, int diskNo, int sectNo,int readwirte);

void writePrinter(char *virtAddr, int len);
void WPnext(int dev_num);

void terminate();

void delayDemon();
int delay_need2unblock(int i);
void delayBlock(int secCnt, state_t *state);
void delayUnblock(int to_unblock);
int getFirstDelayTableFree();
void initDelay();
void initSem();
void initDevices();
void init_base_dev();

void initDiskBuffer();
void initDisk();
void initSyscall();
int getSemIndex(int *virsem);
void setDeviceRegister(int IntlineNo , int DevNo,unsigned int STATUS,unsigned int COMMAND,unsigned int DATA0,unsigned int DATA1,int bitmap);
void getDeviceStatus(int IntlineNo , int DevNo, int* STATUS);
void getDeviceData1(int IntlineNo , int DevNo, int* DATA1);
void getDeviceRegisterHig(int IntlineNo, int DevNo,unsigned int** device);
void diskNextStep(int deviceNo);
void softBlock(pcb_t *pcb);
void unsoftblock(pcb_t *p);
void trasformSectNo(int diskNo, int *cyl_num, int *head_num, int *sect_num, int sectNo);
int min(int a, int b);
#endif
