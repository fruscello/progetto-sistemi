
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
		segT->entry[ASID].Kseg0=&Kseg0;
		segT->entry[ASID].Useg2=&(Useg2[ASID]);
		segT->entry[ASID].Useg3=&Useg3;
		
		
	}else{
		segT->entry[stegTableTop].Kseg0=&Kseg0;
		segT->entry[stegTableTop].Useg2=0;
		segT->entry[stegTableTop].Useg3=&Useg3;
		stegTableTop++;
		
	}
}
void initPTE(PTE* p,int segno,int vpn,int ASID){
	//int i;
	//for(i=0;i<MAXPAGES;i++){
		p->Hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vpn),segno);
		if(segno==0)
			p->Low=ENTRYLO_PFN_SET(0,vpn) | ENTRYLO_VALID | ENTRYLO_DIRTY | ENTRYLO_GLOBAL;
		else if(segno==3)
			p->Low=ENTRYLO_GLOBAL;
		else
			p->Low=0;
	//}
	
}
void initKPT(KPT* p){
	int i;
	for(i=0;i<KSEG0_PT_SIZE;i++){
		int vp=ROMF_EXCVBASE+i*PAGESIZE;
		initPTE(&(p->entry[i]),2,vp,0);
	}
	p->header=(PTE_MAGICNO<<24)|KSEG0_PT_SIZE;
	
}
void initPT2(PT* p,int ASID){
	int i;
	for(i=0;i<MAXPAGES;i++){
		int vp=i*PAGESIZE;
		initPTE(&(p->entry[i]),2,vp,ASID);
		
	}
	p->header=(PTE_MAGICNO<<24)|MAXPAGES;
	
}
void initPT3(PT* p){
	int i;
	for(i=0;i<MAXPAGES;i++){
		int vp=i*PAGESIZE;
		initPTE(&(p->entry[i]),3,vp,0);
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
	next_index=0;
	tlb_mutex=1;
	old_array_head=0;			
	old_array_top=0;
	segT=(segTable *)SEGTABLE_START;		//inizializzo il segment table
	//Useg3.header=PTE_MAGICNO<<24;		//metto il magic number, il EntryCNT rimane a 0
	
	initKPT(&Kseg0);
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
	KPT *kp;
	switch(segno){
		case 0:
			kp=segT->entry[ASID].Kseg0;
			break;
		case 2:
			p=segT->entry[ASID].Useg2;
			break;
		case 3:
			p=segT->entry[ASID].Useg3;
			break;
	}
	if(segno==0)		
		*pte=&(kp->entry[vpn]);
	else
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
	
	pte->Low=ENTRYLO_PFN_SET(pte->Low,RAM_TOP/(FRAMESIZE*WS)-2-next_page_pool) | ENTRYLO_VALID | ENTRYLO_DIRTY;//ATTENZIONE da sistemare, forse potrebbe essere fatto male
	//pte->Low=pte->Hi | ENTRYLO_VALID;
	pagePool[next_page_pool]=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vpn),segno)|1;
	
}
void updateTLB(int segno,int vpn, int ASID){		//aggiorno il tlb con i nuovi valori
	
	PTE *pte;
	findPTE(&pte,segno,vpn,ASID);
	//a_pippo();
	setEntryHi(pte->Hi);
	setEntryLo(pte->Low);
	a_debug[0]=pte->Hi;
	a_debug[1]=pte->Low;
	a_debug[2]=resolving_ASID;
	a_debug[3]=resolving_vpn;
	a_debug[4]=resolving_segno;
	a_debug[5]=ENTRYHI_ASID_GET(pte->Hi);
	if(resolving_segno==0) tprint("resolving_segno == 0(updateTLB, in VM)\n");
	tprint("sto modificando l'indice(updateTLB)\n");
	//TLBWR();
	TLBWI_next();
	//a_pippo();
	
}
void TLBWI_next(){
	int index=next_index;
	a_debug[0]=index;
	setTLB_Index(index<<8);
	next_index=(next_index+1)%TLB_DIM;
	TLBWI();
}
int chuckIfPresent(int segno,int vpn, int ASID){
	//tprint("in chuckIfPresent\n");
	int i=0;
	while(i<TLB_DIM){
		int index=i<<8;
		setTLB_Index(index);
		TLBR();
		int hi=getEntryHi();
		int low=getEntryLo();
		int segno_tlb = ENTRYHI_SEGNO_GET(hi);
		int vpn_tlb = ENTRYHI_VPN_GET(hi);
		int ASID_tlb = ENTRYHI_ASID_GET(hi);
		if((segno_tlb==segno)&&(vpn_tlb==vpn)&&(low&ENTRYLO_VALID)){
			/*if(segno==0){
				hi=ENTRYHI_ASID_SET(hi,ASID);
				setEntryHi(hi);
				TLBWI();
				return 1;
			}else */if(low&ENTRYLO_GLOBAL)
				return 1;
			else if(ASID_tlb==ASID)
				return 1;
		}
		
		a_debug[i]=hi;
		i++;
		//a_pippo();
	}
	return 0;	//se non ha trovato una pagina adatta nel tlb, allora restituisce false
	//a_pippo();
}
void tlbHighHandler(){
	tprint("in VMHandler!\n");
	a_debug[0]=CAUSE_EXCCODE_GET(new_old_state_t[0].CP15_Cause);//getBadVAddr();
	a_debug[1]=ENTRYHI_ASID_GET(new_old_state_t[0].CP15_EntryHi);
	runningPcb->p_s=new_old_state_t[0];
	//a_pippo();
	int cause=CAUSE_EXCCODE_GET(new_old_state_t[0].CP15_Cause);
	if((cause=UTLBLEXCEPTION)||(cause=UTLBSEXCEPTION)||(cause=TLBLEXCEPTION)||(cause=TLBSEXCEPTION)){
		
		old_array[old_array_head]=new_old_state_t[0];
		old_array_top=(old_array_top+1)%MAXUPROC;
		if(old_array_top==old_array_head)
			tprint("errore, coda tlb piena!!!\n");
		
		//tprint("prima SEMP (tlb handler)\n");
		SYSCALL(SEMP, (int)&tlb_mutex, 0, 0);
		//tprint("dopo SEMP (tlb handler)\n");
		
		current_old=old_array[old_array_head];		//si poteva evitare di usare un'altra variabile, ma cosi e' piu chiaro
		old_array_head=(old_array_head+1)%MAXUPROC;
		
		int segno=ENTRYHI_SEGNO_GET(current_old.CP15_EntryHi);
		int vpn=ENTRYHI_VPN_GET(current_old.CP15_EntryHi);
		int ASID=ENTRYHI_ASID_GET(current_old.CP15_EntryHi);
		if(!chuckIfPresent(segno,vpn,ASID)){
			if(segno==0){
				int hi,low;
				hi=ENTRYHI_SEGNO_SET(ENTRYHI_VPN_SET(ENTRYHI_ASID_SET(0,ASID),vpn),segno);
				low=ENTRYLO_PFN_SET(0,vpn)|ENTRYLO_DIRTY|ENTRYLO_VALID;
				setEntryHi(hi);
				setEntryLo(low);
				//TLBWR();
				TLBWI_next();
				//TLBWI();
				//se la v fa andare un altro processo, non fara' mai schedule(&current_old), ma carichera' direttamente lo stato giusto
				V(&tlb_mutex,&(runningPcb->p_s));		
				schedule(&current_old);
				//ferma l'esecuzione e fa andare di nuovo il pcb corrente
			}
			requesting_tlb=runningPcb;
			
			
			a_debug[0]=segno;
			a_debug[1]=vpn;
			a_debug[2]=ASID;
			resolving_ASID=ASID;
			resolving_vpn=vpn;
			resolving_segno=segno;
			/*state_t s;				//per verificare che gli interrupt siano gia disabilitati
			STST(&s);
			a_debug[0]=s.cpsr;
			a_debug[1]=STATUS_ALL_INT_ENABLE(s.cpsr);
			a_debug[2]=STATUS_ALL_INT_DISABLE(s.cpsr);
			a_pippo();*/
			
			
			//ATTENZIONE	disabilita gli interrupt!!! (sembrerebbero gia disabilitati)
			
			PTE *p;
			findPTE(&p,segno,vpn,ASID);
			if(p->Low&ENTRYLO_VALID){		//se la pagina e' gia valida la carichi solo nel tlb
				tprint("pagina gia' valida! (tlbHandler)\n");
				updateTLB(segno,vpn,ASID);
			}else{
			
				if(pagePool[next_page_pool]&1){			//mi chiesto se la pagina sta venendo utilizzata
					tprint("SI pagePool[next_page_pool]&1 (tlb handler)\n");
					setInvalid( segno, ENTRYHI_VPN_GET(pagePool[next_page_pool]), ENTRYHI_SEGNO_GET(pagePool[next_page_pool]) );
					TLBCLR();
					SYSCALL(DISK_PUT, RAM_TOP - 2*PAGESIZE-next_page_pool*PAGESIZE, 0, /*ASID*MAXPAGES+vpn*/0);		//rimetto la pagina su disco
					
					
				}else{
					tprint("NO pagePool[next_page_pool]&1 (tlb handler)\n");
					//tlbNextStep();
					
				}
				SYSCALL(DISK_GET, RAM_TOP - 2*FRAMESIZE*WS-next_page_pool*FRAMESIZE*WS, 0, /*ASID*MAXPAGES+vpn*/0);		//da sistemare il caso di memoria globlale
				tprint("dopo diskget(tlbhandler)\n");
				setValid(resolving_segno,resolving_vpn,resolving_ASID);			//aggiorno le PTE e le strutture per la gestione del page pool
				updateTLB(resolving_segno,resolving_vpn,resolving_ASID);
				next_page_pool=(next_page_pool+1)%PAGE_POOL_SIZE;
			}
		}else{
			a_debug[0]=segno;
			a_debug[1]=vpn;
			a_debug[2]=ASID;
			//a_pippo();
		}
		V(&tlb_mutex,&(requesting_tlb->p_s));
		//requesting_tlb->p_s=current_old;
		tprint("faccio lo scheduling\n");
		schedule(&current_old);
		
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
		requesting_tlb->p_s=current_old;
		schedule(&(requesting_tlb->p_s));
	}
}
