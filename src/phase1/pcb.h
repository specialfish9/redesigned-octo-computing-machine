#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

extern void initPcbs(void);

extern void freePcb(pcb_t *);

extern pcb_t *allocPcb(void);

extern void mkEmptyProcQ(struct list_head *);

extern int emptyProcQ(struct list_head *);

extern void insertProcQ(struct list_head *, pcb_t *);

extern pcb_t *headProcQ(struct list_head *);

extern pcb_t *removeProcQ(struct list_head *);

extern pcb_t *outProcQ(struct list_head *, pcb_t *);

#endif
