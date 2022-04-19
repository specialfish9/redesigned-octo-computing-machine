/*********************************pcb.h****************************************
 *
 *  Dichiarazione delle funzioni che gestiscono la coda e l'albero dei PCB.
 *
 ******************************************************************************/

#ifndef PCB_H
#define PCB_H

#include "pandos_types.h"

/* UTILITY SUI PCB */

/**
 * @brief Ritorna un pcb dato il suo PID.
 * @param Il pid
 * @return Il pcb associato
 * */
extern pcb_t *search_by_pid(const unsigned int);

/**
 * @brief Ritorna TRUE se il pcb passato come parametro e' ancora vivo, FALSE
 * altrimenti
 * @param pcb Il pcb da controllare
 * @returns TRUE o FALSE
 * */
extern int is_alive(const pcb_t *const pcb);

/* CODA DEI PCB */

/** 
 * @brief Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table.
 * 
 * */
extern void init_pcbs(void);

/**
 * @brief Trasferisce il descrittore puntato da p nella lista dei PCB liberi
 * 
 * @param p il pcb del processo da aggiungere a pcbFree
 * 
 */
extern void free_pcb(pcb_t *);

/**
 * @brief Restituisce NULL se pcbFree_h è vuota. Altrimenti rimuove un elemento da
 * pcbFree, inizializza tutti i campi (NULL/0) e restituisce l’elemento
 * rimosso.
 * 
 * @return Il pcb appena allocato, o NULL se non sono disponibili pcb liberi 
 */
extern pcb_t *alloc_pcb(void);

/**
 * @brief Crea una lista di PCB, inizializzandola come lista vuota
 * 
 * @param head Il puntatore alla lista da inizializzare
 * 
 */
extern void mk_empty_proc_q(struct list_head *);

/**
 * @brief Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.
 * 
 * @param head Puntatore alla lista da controllare
 * 
 * @return TRUE se la lista puntata da head è vuota, FALSE altrimenti.
 */
extern int empty_proc_q(struct list_head *);

/**
 * @brief Inserisce l’elemento puntato da p nella coda dei processi puntata da head.
 * 
 * @param head Puntatore alla lista in cui va aggiunto il pcb
 * @param p PCB da aggiungere alla lista
 * 
 */
extern void insert_proc_q(struct list_head *, pcb_t *);

/**
 * @brief  Restituisce il primo elemento nella lista. Se la lista è vuota il risultato è
 * NULL.
 * 
 * @param head Puntatore alla lista di cui va restituito il primo elemento
 * 
 * @return NULL se la lista è vuota, altrimenti il puntatore al primo pcb in lista 
 */
extern pcb_t *head_proc_q(struct list_head *);

/**
 * @brief Rimuove il primo elemento presente nella lista data. Se la lista è vuota il
 * risultato è NULL.
 * 
 * @param head Puntatore alla lista da cui estrarre il primo elemento
 * 
 * @return NULL se la lista è vuota, altrimenti il puntatore al pcb rimosso 
 */
extern pcb_t *remove_proc_q(struct list_head *);

/**
 * @brief Elimina il pcb "p" dalla lista data e lo restituisce. Se p non è presente, il
 * risultato è NULL.
 * 
 * @param head la lista cui va eliminato il pcb
 * @param p il pcb da eliminare e restituire
 * 
 * @return NULL se p non è presente, altrimenti p
 */
extern pcb_t *out_proc_q(struct list_head *, pcb_t *);

/* ALBERO DEI PCB */

/**
 * @brief Restituisce TRUE se il PCB puntato da p non ha figli, FALSE altrimenti. 
 * 
 * @param p il PCB da controllare
 * @return TRUE se p non ha figli, FALSE altrimenti
 */
extern const int empty_child(const pcb_t *p);

/**
 * @brief  Inserisce il PCB puntato da p come figlio del PCB puntato da prnt. 
 * 
 * @param prnt Puntatore al PCB del processo padre
 * @param p Puntatore al PCB del processo figlio
 */
extern void insert_child(pcb_t *prnt, pcb_t *p);

/**
 * @brief Rimuove il primo figlio del PCB puntato da p. Se p non ha figli,
 * restituisce NULL 
 * 
 * @param p Puntatore al PCB da cui va rimosso il primo figlio
 * @return NULL se p non ha figli, altrimenti il pcb del primo figlio
 */
extern pcb_t *remove_child(pcb_t *p);

/**
 * @brief  Rimuove il PCB puntato da p dalla lista dei figli del padre.  A differenza della removeChild, p può trovarsi
 * in una posizione arbitraria (ossia non è necessariamente il primo figlio del
 * padre).
 * 
 * @param p il processo da rimuovere dalla lista dei figli del proprio processo padre.
 * @return NULL se il PCB puntato da p non ha un padre, altrimenti restituisce l’elemento rimosso.
 */
extern pcb_t *out_child(pcb_t *p);

#endif
