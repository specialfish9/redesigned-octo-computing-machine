/**
 *
 * @file scheduler.h
 * @brief Definizioni delle funzioni implementate dallo scheduler. Lo scheduler
 * si organizza i processi in stato di ready a seconda della loro priorità.
 * In paricolare si occupa di creare, mandare in esecuzione ed uccidere i
 * processi.
 *
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pandos_types.h"

/** Puntatore al pcb del processo attivo */
extern pcb_t *act_proc;
/** Numero di processi soft blocked*/
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
 * @brief Carica il processo passato come parametro. Utilizzare con estrema
 * cautela.
 * @param pcb Riferimento del processo
 * */
extern void load_proc(pcb_t *pcb);

extern void load_with_state(pcb_t *pcb, state_t *state);
#endif
