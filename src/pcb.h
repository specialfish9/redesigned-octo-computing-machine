/*********************************pcb.h****************************************
 *
 *  Dichiarazione delle funzioni che gestiscono la coda e l'albero dei PCB.
 *
 ******************************************************************************/

#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/**
 * @brief Ritorna un pcb dato il suo PID.
 * @param Il pid
 * @return Il pcb associato
 * */
extern pcb_t *search_by_pid(const unsigned int);

/* CODA DEI PCB */

/* Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 * pcbFree_table.*/
extern void init_pcbs(void);

/* Inserisce il PCB puntato da p nella lista dei PCB liberi.*/
extern void free_pcb(pcb_t *);

/* Restituisce NULL se pcbFree_h è vuota. Altrimenti rimuove un elemento da
 * pcbFree, inizializza tutti i campi (NULL/0) e restituisce l’elemento
 * rimosso.*/
extern pcb_t *alloc_pcb(void);

/* Crea una lista di PCB, inizializzandola come lista vuota */
extern void mk_empty_proc_q(struct list_head *);

/* Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.*/
extern int empty_proc_q(struct list_head *);

/*Inserisce l’elemento puntato da p nella coda dei processi puntata da head.*/
extern void insert_proc_q(struct list_head *, pcb_t *);

/* Restituisce il primo elemento nella lista. Se la lista è vuota il risultato è
 * NULL.*/
extern pcb_t *head_proc_q(struct list_head *);

/* Rimuove il primo elemento presente nella lista data. Se la lista è vuota il
 * risultato è NULL.*/
extern pcb_t *remove_proc_q(struct list_head *);

/* Elimina il pcb "p" dalla lista data e lo restituisce. Se p non è presente, il
 * risultato è NULL. */
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
