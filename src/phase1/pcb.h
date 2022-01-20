#ifndef PCB_H
#define PCB_H
#include "pandos_const.h"
#include "pandos_types.h"

static pcb_t pcbFree_table[MAXPROC];
static pcb_t *pcbFree_h;

/* LIST FUNCTIONS */

void initPcbs(void);

/* TREES FUNCTIONS */

/* Returns TRUE if the PCB pointed by p hasn't got any child. FALSE otherwise */
const int emptyChild(const pcb_t *p);

/* Inserts the PCB pointed by p as a child of the PCB pointed by prnt */
void insertChild(pcb_t *prnt, pcb_t *p);

/* Removes the first child of the PCB pointed by p. If p hasn't children
 * returns NULL*/
pcb_t *removeChild(pcb_t *p);

/*
 * Removes the PCB pointed by p from the list of children of his father.
 * If the PCB pointed by p hasn't a father then returns NULL, otherwise returns
 * the deleted element (p). p can be in every position, it may not be the first
 * child .
 */
pcb_t *outChild(pcb_t *p);

#endif
