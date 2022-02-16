#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/* FUNZIONI LISTA DEI PCB */

extern void initPcbs(void);

extern void freePcb(pcb_t *);

extern pcb_t *allocPcb(void);

extern void mkEmptyProcQ(struct list_head *);

extern int emptyProcQ(struct list_head *);

extern void insertProcQ(struct list_head *, pcb_t *);

extern pcb_t *headProcQ(struct list_head *);

extern pcb_t *removeProcQ(struct list_head *);

extern pcb_t *outProcQ(struct list_head *, pcb_t *);

/* FUNZIONI ALBERO DEI PCB */

/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. */
extern const int emptyChild(const pcb_t *p);

/* Inserisce il PCB puntato da p come figlio del PCB puntato da prnt. */
extern void insertChild(pcb_t *prnt, pcb_t *p);

/* Rimuove il primo figlio del PCB puntato da p. Se p non ha figli,
 * restituisce NULL */
extern pcb_t *removeChild(pcb_t *p);

/* Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB 
* puntato da p non ha un padre, restituisce NULL, altrimenti restituisce 
* l’elemento rimosso (cioè p). A differenza della removeChild, p può trovarsi 
* in una posizione arbitraria (ossia non è necessariamente il primo figlio del 
* padre).*/
extern pcb_t *outChild(pcb_t *p);

#endif
