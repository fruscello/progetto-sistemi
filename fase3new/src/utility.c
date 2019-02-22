#include <utility.h>
#include <libuarm.h>
#include <types.h>
#include <arch.h>

void a_pippo(){}
void stateCpy(state_t *out,state_t *in){
	out->a1=in->a1;
	out->a2=in->a2;
	out->a3=in->a3;
	out->a4=in->a4;
	out->v1=in->v1;
	out->v2=in->v2;
	out->v3=in->v3;
	out->v4=in->v4;
	out->v5=in->v5;
	out->v6=in->v6;
	out->sl=in->sl;
	out->fp=in->fp;
	out->ip=in->ip;
	out->sp=in->sp;
	out->lr=in->lr;
	out->pc=in->pc;
	out->cpsr=in->cpsr;
	out->CP15_Control=in->CP15_Control;
	out->CP15_EntryHi=in->CP15_EntryHi;
	out->CP15_Cause=in->CP15_Cause;
	out->TOD_Hi=in->TOD_Hi;
	out->TOD_Low=in->TOD_Low;
	tprint("in stateCpy\n");
}
