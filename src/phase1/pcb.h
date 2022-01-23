#ifndef PCB_H
#define PCB_H
#include "pandos_const.h"
#include "pandos_types.h"

static pcb_t pcbFree_table[MAX_PROC];
static struct list_head pcbFree_h;

void initPcbs(void);
void freePcb(pcb_t *);
pcb_t *allocPcb(void);
void mkEmptyProcQ(list_head *);
int emptyProcQ(list_head *);
void insertProcQ(list_head *, pcb_t *);
pcb_t *headProcQ(list_head *);
pcb_t *removeProcQ(list_head *);
pcb_t *outProcQ(list_head *, pcb_t *);
#endif
