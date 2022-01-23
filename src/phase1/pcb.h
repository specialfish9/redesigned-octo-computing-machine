#ifndef PCB_H
#define PCB_H
#include "pandos_types.h"
#include "pandos_const.h"

static pcb_t pcbFree_table[MAX_PROC]; 
static struct  list_head pcbFree_h;

void initPcbs(void);

#endif
