#ifndef PCB_H
#define PCB_H
#include "pandos_const.h"
#include "pandos_types.h"

static pcb_t pcbFree_table[MAXPROC];
static struct list_head *pcbFree_h;

void initPcbs(void);
void freePcb(pcb_t *);
pcb_t *allocPcb(void);
void mkEmptyProcQ( struct list_head *);
int emptyProcQ(struct list_head *);
void insertProcQ(struct list_head *, pcb_t *);
pcb_t *headProcQ(struct list_head *);
pcb_t *removeProcQ(struct list_head *);
pcb_t *outProcQ(struct list_head *, pcb_t *);
#endif
