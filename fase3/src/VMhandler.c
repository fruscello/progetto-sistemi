
#include <highhandler.h>
#include <VMhandler.h>
#include <utility.h>
#include <arch.h>
#include <uARMconst.h>
#include <uARMtypes.h>
#include <libuarm.h>
#include <init3.h>
#include <highsyscall.h>
#include <scheduler.h>
#include <syscall.h>
#include <pcb.h>
#include <main.h>

void initSegT(int ASID){
	if ((ASID<8)&&(ASID>=0)){		//inizializzo la segment table per il processo
		segT->entry[ASID].Kseg0=0;
		segT->entry[ASID].Useg2=&(Useg2[ASID]);
		segT->entry[ASID].Useg3=&Useg3;
		
		
	}else{
		segT->entry[stegTableTop].Kseg0=0;
		segT->entry[stegTableTop].Useg2=0;
		segT->entry[stegTableTop].Useg3=&Useg3;
		stegTableTop++;
		
	}
}
void initPTE(PTE* p,int segno,int vp,int ASID){
	int i;
	for(i=0;i<MAXPAGES;i++){
		p->Hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vp),segno);
	}
	
}
void initPT2(PT* p,int ASID){
	int i;
	for(i=0;i<MAXPAGES;i++){
		int vp=i*FRAMESIZE;
		//p->entry[i].Hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vp),segno);
		initPTE(&(p->entry[i]),2,vp,ASID);
		p->entry[i].Low=0;
	}
	p->header=(PTE_MAGICNO<<24)|MAXPAGES;
	
}
void initPT3(PT* p){
	int i;
	for(i=0;i<MAXPAGES;i++){
		int vp=i*FRAMESIZE;
		//p->entry[i].Hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vp),segno);
		initPTE(&(p->entry[i]),3,vp,0);
		p->entry[i].Low=0|ENTRYLO_GLOBAL;
	}
	p->header=(PTE_MAGICNO<<24)|MAXPAGES;
	
}
void eliminateASID(){
	
}
void initPagePool(){
	int i;
	for (i=0;i<PAGE_POOL_SIZE;i++){
		pagePool[i]=0;
		
	}
}
void initTlbHandler(){
	tlb_step=0;
	stegTableTop=8;
	next_page_pool=10;
	tlb_mutex=1;
	segT=(segTable *)SEGTABLE_START;		//inizializzo il segment table
	//Useg3.header=PTE_MAGICNO<<24;		//metto il magic number, il EntryCNT rimane a 0
	int i;
	for(i=0;i<8;i++){			//inizializzo staticamente i primi 8 ASID come gli 8 processi utente
			
		initPT2(&(Useg2[i]),i);
		initSegT(i);
	}
	initPT3(&Useg3);
	initPagePool();
	//initPT(Useg2[i]);			//da inizializzare la PT del kernel
	
	/*PT *U3PageTable;
	U3PageTable[i]->hi...*/
}
void findPTE(PTE** pte,int segno,int vpn, int ASID){
	PT *p;
	switch(segno){
		case 0:
			p=segT->entry[ASID].Kseg0;
			break;
		case 2:
			p=segT->entry[ASID].Useg2;
			break;
		case 3:
			p=segT->entry[ASID].Useg3;
			break;
	}
	*pte=&(p->entry[vpn]);
}
void setInvalid(int segno,int vpn, int ASID){
	/*PT* p;
	switch(segno){
		case 0:
			p=segT->entry[ASID].Kseg0;
			break;
		case 2:
			p=segT->entry[ASID].Useg2;
			break;
		case 3:
			p=segT->entry[ASID].Useg3;
			break;
	}*/
	PTE *pte;
	findPTE(&pte,segno,vpn,ASID);
	
	pte->Low=pte->Low&(!ENTRYLO_VALID);
	
}
void setValid(int segno,int vpn, int ASID){		//setto a valido la pte e aggiorno la page pool indicando la presenza del nuovo momento
	/*PT* p;
	switch(segno){
		case 0:
			p=segT->entry[ASID].Kseg0;
			break;
		case 2:
			p=segT->entry[ASID].Useg2;
			break;
		case 3:
			p=segT->entry[ASID].Useg3;
			break;
	}
	p->entry[vpn].Hi=p->entry[vpn].Hi | ENTRYLO_VALID;*/
	PTE *pte;
	findPTE(&pte,segno,vpn,ASID);
	
	pte->Low=ENTRYLO_PFN_SET(pte->Low,RAM_TOP/FRAMESIZE-2-next_page_pool) | ENTRYLO_VALID | ENTRYLO_DIRTY;//ATTENZIONE da sistemare, forse potrebbe essere fatto male
	//pte->Low=pte->Hi | ENTRYLO_VALID;
	pagePool[next_page_pool]=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vpn),segno)|1;
	
}
void updateTLB(int segno,int vpn, int ASID){		//aggiorno il tlb con i nuovi valori
	
	PTE *pte;
	findPTE(&pte,segno,vpn,ASID);
	
	a_pippo();
	setEntryHi(pte->Hi);
	setEntryLo(pte->Low);
	a_debug[0]=pte->Hi;
	a_debug[1]=pte->Low;
	a_debug[2]=resolving_ASID;
	a_debug[3]=resolving_vpn;
	a_debug[4]=resolving_segno;
	if(resolving_segno==0) tprint("resolving_segno == 0(updateTLB, in VM)\n");
	TLBWR();
	a_pippo();
	
}
void tlbHighHandler(){
	tprint("in VMHandler!\n");
	//new_old_state_t[1].CP15_Control=CP15_ENABLE_VM(new_old_state_t[1].CP15_Control);
	a_debug[0]=CAUSE_EXCCODE_GET(new_old_state_t[0].CP15_Cause);//getBadVAddr();
	a_debug[1]=ENTRYHI_ASID_GET(new_old_state_t[0].CP15_EntryHi);
	runningPcb->p_s=new_old_state_t[0];
	/*setTLB_Index(1);
	setEntryHi(16);
	setEntryLo(4);*/
	//TLBWR();
	//setEntryHi(1);
	//int entry_hi=getEntryHi();			//ENTRYHI_ASID_GET(new_old_state_t[1].CP15_EntryHi);
	//if(entry_hi==1) tprint("entry_hi=1\n");
	
	int cause=CAUSE_EXCCODE_GET(new_old_state_t[0].CP15_Cause);
	if((cause=UTLBLEXCEPTION)||(cause=UTLBSEXCEPTION)||(cause=TLBLEXCEPTION)||(cause=TLBSEXCEPTION)){
		int segno=ENTRYHI_SEGNO_GET(new_old_state_t[0].CP15_EntryHi);
		if(segno==0){
			int hi,low;
			int vpn=ENTRYHI_VPN_GET(new_old_state_t[0].CP15_EntryHi);
			int ASID=ENTRYHI_ASID_GET(new_old_state_t[0].CP15_EntryHi);
			hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vpn),segno);
			low=ENTRYLO_PFN_SET(0,vpn)|ENTRYLO_DIRTY|ENTRYLO_VALID;
			setEntryHi(hi);
			setEntryLo(low);
			TLBWR();
			//a_pippo();
			schedule(&(runningPcb->p_s));
		}
		tprint("prima SEMP (tlb handler)\n");
		SYSCALL(SEMP, (int)&tlb_mutex, 0, 0);
		requesting_tlb=runningPcb;
		old_VM=new_old_state_t[0];
		tprint("dopo SEMP (tlb handler)\n");
		segno=ENTRYHI_SEGNO_GET(new_old_state_t[0].CP15_EntryHi);
		int vpn=ENTRYHI_VPN_GET(new_old_state_t[0].CP15_EntryHi);
		int ASID=ENTRYHI_ASID_GET(new_old_state_t[0].CP15_EntryHi);
		
		
		
		
		a_debug[0]=segno;
		a_debug[1]=vpn;
		a_debug[2]=ASID;
		//a_pippo();
		resolving_ASID=ASID;
		resolving_vpn=vpn;
		resolving_segno=segno;
		/*state_t s;				//per verificare che gli interrupt siano gia disabilitati
		STST(&s);
		a_debug[0]=s.cpsr;
		a_debug[1]=STATUS_ALL_INT_ENABLE(s.cpsr);
		a_debug[2]=STATUS_ALL_INT_DISABLE(s.cpsr);
		a_pippo();*/
		
		//ATTENZIONE	verifica se la pagina manca ancora
		//ATTENZIONE	disabilita gli interrupt!!!
		tlb_step=2;
		if(pagePool[next_page_pool]&1){			//mi chiesto se la pagina sta venendo utilizzata
			tprint("SI pagePool[next_page_pool]&1 (tlb handler)\n");
			setInvalid( segno, ENTRYHI_VPN_GET(pagePool[next_page_pool]), ENTRYHI_SEGNO_GET(pagePool[next_page_pool]) );
			TLBCLR();
			SYSCALL(DISK_PUT, RAM_TOP - 2*FRAMESIZE-next_page_pool*FRAMESIZE, 0, ASID);		//rimetto la pagina su disco
			
		}else{
			tprint("NO pagePool[next_page_pool]&1 (tlb handler)\n");
			tlbNextStep();
			
		}
		
	}else{
		tprint("errore non previsto!! (tlbHighHandler)\n");
	}
	tprint("fine tbl handler\n");
	//while(1){}
	
			
	schedule(&new_old_state_t[0]);
	
}
void tlbNextStep(){
	if(tlb_step==2){
		tprint("tlb_step=2(tlbNextStep)\n");
		tlb_step=3;
		a_debug[5]=RAM_TOP - 2*FRAMESIZE-next_page_pool*FRAMESIZE;
		SYSCALL(DISK_GET, RAM_TOP - 2*FRAMESIZE-next_page_pool*FRAMESIZE, 0, resolving_ASID);		//prendo la pagina richiesta dal disco e la metto in ram
	}else{
		tprint("tlb_step=3(tlbNextStep)\n");
		
		setValid(resolving_segno,resolving_vpn,resolving_ASID);			//aggiorno le PTE e le strutture per la gestione del page pool
		
		updateTLB(resolving_segno,resolving_vpn,resolving_ASID);
		
		//TLBCLR();
		next_page_pool=(next_page_pool+1)%PAGE_POOL_SIZE;
		tlb_step=0;
		unsoftblock(requesting_tlb);
		//a_pippo();
		V(&tlb_mutex,&(requesting_tlb->p_s));
		V(&device_mutex[0][DISK_MUTEX],&(requesting_tlb->p_s));
			if(activePcbs==0)tprint("activePcbs==0 (tlb)\n");
			if(softBlockedPcbs==0)tprint("softBlockedPcbs==0 (tlb)\n");
		requesting_tlb->p_s=old_VM;
		schedule(&(requesting_tlb->p_s));
	}
}
