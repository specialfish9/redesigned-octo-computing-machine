#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/* LIST FUNCTIONS */

/* Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 * pcbFree_table.*/
extern void initPcbs(void);

/* Inserisce il PCB puntato da p nella lista dei PCB liberi.*/
extern void freePcb(pcb_t *);

/* Restituisce NULL se pcbFree_h è vuota. Altrimenti rimuove un elemento da
 * pcbFree, inizializza tutti i campi (NULL/0) e restituisce l’elemento
 * rimosso.*/
extern pcb_t *allocPcb(void);

/* Crea una lista di PCB, inizializzandola come lista vuota */
extern void mkEmptyProcQ(struct list_head *);

/* Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.*/
extern int emptyProcQ(struct list_head *);

/*Inserisce l’elemento puntato da p nella coda dei processi puntata da head.*/
extern void insertProcQ(struct list_head *, pcb_t *);

/* Restituisce il primo elemento nella lista. Se la lista è vuota il risultato è
 * NULL.*/
extern pcb_t *headProcQ(struct list_head *);

/* Rimuove il primo elemento presente nella lista data. Se la lista è vuota il
 * risultato è NULL.*/
extern pcb_t *removeProcQ(struct list_head *);

/* Elimina il pcb "p" dalla lista data e lo restituisce. Se p non è presente, il
 * risultato è NULL. */
extern pcb_t *outProcQ(struct list_head *, pcb_t *);

/* TREES FUNCTIONS */

/* Returns TRUE if the PCB pointed by p hasn't got any child. FALSE otherwise */
extern const int emptyChild(const pcb_t *p);

/* Inserts the PCB pointed by p as a child of the PCB pointed by prnt */
extern void insertChild(pcb_t *prnt, pcb_t *p);

/* Removes the first child of the PCB pointed by p. If p hasn't children
 * returns NULL*/
extern pcb_t *removeChild(pcb_t *p);

/*
 * Removes the PCB pointed by p from the list of children of his father.
 * If the PCB pointed by p hasn't a father then returns NULL, otherwise returns
 * the deleted element (p). p can be in every position, it may not be the first
 * child .
 */
extern pcb_t *outChild(pcb_t *p);

#endif
