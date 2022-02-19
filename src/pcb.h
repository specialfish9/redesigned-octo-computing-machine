/*********************************pcb.h****************************************
 *
 *  Dichiarazione delle funzioni che gestiscono la coda e l'albero dei PCB.
 *
 ******************************************************************************/

#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/* CODA DEI PCB */

extern void init_pcbs(void);

extern void free_pcb(pcb_t *);

extern pcb_t *alloc_pcb(void);

extern void mk_empty_proc_q(struct list_head *);

extern int empty_proc_q(struct list_head *);

extern void insert_proc_q(struct list_head *, pcb_t *);

extern pcb_t *head_proc_q(struct list_head *);

extern pcb_t *remove_proc_q(struct list_head *);

extern pcb_t *out_proc_q(struct list_head *, pcb_t *);

/* ALBERO DEI PCB */

/* Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. */
extern const int empty_child(const pcb_t *p);

/* Inserisce il PCB puntato da p come figlio del PCB puntato da prnt. */
extern void insert_child(pcb_t *prnt, pcb_t *p);

/* Rimuove il primo figlio del PCB puntato da p. Se p non ha figli,
 * restituisce NULL */
extern pcb_t *remove_child(pcb_t *p);

/* Rimuove il PCB puntato da p dalla lista dei figli del padre. Se il PCB
 * puntato da p non ha un padre, restituisce NULL, altrimenti restituisce
 * l’elemento rimosso (cioè p). A differenza della removeChild, p può trovarsi
 * in una posizione arbitraria (ossia non è necessariamente il primo figlio del
 * padre).*/
extern pcb_t *out_child(pcb_t *p);

#endif
