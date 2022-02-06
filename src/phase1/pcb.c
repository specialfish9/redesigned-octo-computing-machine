#include "pcb.h"
#include "listx.h"

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table. 
 */
void initPcbs(void){
	pcbFree_h->p_list =LIST_HEAD_INIT(pcbFree_h->p_list);

	for(int i = 0; i < MAX_PROC; i++) {
	  list_add(pcbFree_table[i], pcbFree_h->p_list);
	}
	
	for (pcb_t* ptr = pcbFree_table; ptr != NULL; ptr ++){
	  list_add(ptr, pcbFree_h->p_list);
	}
}
