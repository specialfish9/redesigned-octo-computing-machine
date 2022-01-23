#ifndef PCB_H
#define PCB_H
#include "pandos_const.h"
#include "pandos_types.h"


void initPcbs(void);
void freePcb(pcb_t *p);
pcb_t *allocPcb(void);
void mkEmptyProcQ(struct list_head *head);
int emptyProcQ (struct list_head *head);



#endif
