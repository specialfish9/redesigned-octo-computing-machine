#include "pcb.h"
#include "listx.h"

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table. 
 */
void initPcbs(void){
	pcbFree_h=LIST_HEAD_INIT(pcbFree_h);

	for(int i = 0; i < MAX_PROC; i++) {
		  list_add( &pcbFree_table[i].p_list, &pcbFree_h);
	}
	
/*	for (pcb_t* ptr = pcbFree_table; ptr != NULL; ptr ++){
	  list_add(ptr, pcbFree_h->p_list);
	}*/
}
void freePcb(pcb_t *p){
	list_del(&p->p_list);
	list_add(&p->p_list, &pcbFree_h);
}

pcb_t *allocPcb(){
	if(list_empty(&pcbFree_h))
		return NULL;
	pcb_t *pcb= container_of(pcbFree_h.next, pcb_t, p_list);
	list_del(pcbFree_h.next);
	pcb->p_parent=NULL;
	pcb->p_s=0;
	pcb->p_time=0;
	pcb->p_semAdd=NULL;
	return pcb;
}
