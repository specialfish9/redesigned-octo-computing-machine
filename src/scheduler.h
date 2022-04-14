#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pandos_types.h"

extern pcb_t *act_proc;
extern size_tt sb_procs;

/**
 * @brief Inizializza le variabili e le strutture dati dello scheduler.
 */
extern void init_scheduler(void);

/**
 * @brief Crea il processo di init
 * @param entry_point l'indirizzo della funzione entry_point che deve avere
 * il processo.
 * */
extern void create_init_proc(const memaddr entry_point);

/**
 * @brief Passa il controllo al prossimo processo seguendo le politiche dello
 * scheduler.
 * */
extern void scheduler_next(void);

/**
 * @brief Crea un processo figlio del processo correntemente in esecuzione.
 * @param statep puntatore allo stato del nuovo processo.
 * @param prio priorita' del nuovo processo.
 * @param supportp puntatore alla struttura di supporto del nuovo processo.
 * @return il pcb del processo creato o NULL se non e' possibile crearlo */
extern pcb_t *mk_proc(state_t *statep, int prio, support_t *supportp);

/**
 * @brief Termina un processo
 * @param p riferimento al processo da terminare.
 * */
extern void kill_proc(pcb_t *p);

/**
 * @brief Aggiunge un processo alla coda ready specificata.
 * @param pcb puntatore al pcb del processo.
 * @param priority priorita' del processo.
 * */
extern void enqueue_proc(pcb_t *const pcb, const unsigned int priority);

/**
 * @brief Rimuove un processo dalla coda ready specificata.
 * @param priority la priorita' della coda dalla quale si vuole rimuovere il
 * pcb.
 * @return il pcb rimosso o NULL se non ci sono pcb in coda
 * */
extern pcb_t *dequeue_proc(const unsigned int priority);

/**
 * @brief Rimuove un processo specifico dalla coda ready indicata.
 * @param priority la priorita' della coda dalla quale si vuole rimuovere il
 * pcb.
 * @param pcb il pcb da rimuovere.
 * @return il pcb rimosso o NULL se non ci sono pcb in coda
 * */
extern pcb_t *rm_proc(pcb_t *const pcb, const unsigned int priority);

#endif
