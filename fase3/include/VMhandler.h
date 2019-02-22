
#ifndef VMHANDLER_H
#define VMHANDLER_H
#define MAXPAGES 32
#define PAGESIZE FRAMESIZE*WS
#define U2SEG 0x80000000
#define U3SEG 0xC0000000
#define MAXUPROC 8
#define PAGE_POOL_SIZE MAXUPROC*2
#define SEG_TABLE_DIM 128
#define TLB_DIM 16
#define SEG_TABLE_POINTER 0x00007600
//#define FRSME_SIZE 4096		gia definito come FRAMESIZE

#include <pcb.h>




typedef struct {
	unsigned int Hi;
	unsigned int Low;
} PTE;

typedef struct {
	unsigned int header;
	PTE entry[MAXPAGES];
} PT;

typedef struct {
	PT *Kseg0;
	PT *Useg2;
	PT *Useg3;
} segTableEntry;

typedef struct {
	segTableEntry entry[SEG_TABLE_DIM];
} segTable;



int stegTableTop;		//indica quante posizioni del segment table sono state occupate
segTable *segT;
PT Useg3;
PT Useg2[MAXUPROC];

state_t old_array[MAXUPROC];
state_t current_old;
int old_array_head;			//qual'e' la prossima posizione da risolvare
int old_array_top;			//quale sara' la prossima posizione da riempire

int tlb_step;				//2 se tlb next step dovra' effettuare il second step, 3 se dovra' effettuare il third step
int next_page_pool;			//indica la prossima pagina da sostituire nella page pool (serve per implementre round robin)
int next_index;
unsigned int pagePool[PAGE_POOL_SIZE];		//ogni elemento e' un entry Hi: contiene segno, VPN e ASID. l'ultimo bit e' 1 se la pagina sta venendo usata
int a_debug[20];			//lo uso solo per fare debug!!!!
int tlb_mutex;
pcb_t *requesting_tlb;			//processo che ha generato la tlb
int resolving_ASID;			//ASID del processo che sta attualmente risolvendo l'eccezione
int resolving_vpn;			//vpn del processo che sta attualmente risolvendo l'eccezione
int resolving_segno;			//segno del processo che sta attualmente risolvendo l'eccezione

void initSegT(int ASID);		//ASID deve essere -1 se se il processo non e' un processo utente
void initPTE(PTE* p,int segno,int vp,int ASID);
void initPT2(PT* p,int ASID);
void initPT3(PT* p);
void eliminateASID();
void initTlbHandler();
void findPTE(PTE** pte,int segno,int vpn, int ASID);
void setInvalid(int segno,int vpn, int ASID);
void setValid(int segno,int vpn, int ASID);
void updateTLB(int segno,int vpn, int ASID);
void TLBWI_next();
int chuckIfPresent(int segno,int vpn, int ASID);
void tlbHighHandler();
void tlbNextStep();

#endif // MAIN_H
