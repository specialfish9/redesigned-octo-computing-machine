/*********************************asl.h****************************************
 *
 *  Dichiarazione delle funzioni che gestiscono la Active Semaphore List (ASL)
 *
 ******************************************************************************/
#ifndef ASL_H
#define ASL_H

#include "pandos_types.h"

semd_t *get_semd(int *s_key);
/**
 @brief Inizializza la ASL.
*/
extern void init_asl(void);

/**
 @brief Inserisce il PCB puntato da p nella coda dei processi bloccati associata
 al parametro semAdd. Se il semaforo corrispondente non è presente nella ASL ne
 alloca uno nuovo dalla lista di quelli liberi.
 @param semAdd chiave del semaforo
 @param p puntatore al PCB del processo da bloccare
 @return TRUE se non è possibile allocare un nuovo SEMD (la lista dei liberi è
 vuota), FALSE altrimenti
*/
extern int insert_blocked(int *semAdd, pcb_t *p);

/**
 @brief Sblocca e rimuove il primo PCB dalla coda dei processi bloccati sul
 semaforo associato a semAdd.
 @param semAdd chiave del semaforo
 @return puntatore al PCB rimosso dalla coda dei bloccati, NULL se il
 descrittore semAdd non esiste nella ASL
*/
extern pcb_t *remove_blocked(int *semAdd);

/**
 @brief Sblocca e rimuove il PCB puntato da p dalla coda del semaforo su cui è
 bloccato.
 @param p puntatore al PCB
 @return puntatore al PCB rimosso dalla coda dei bloccati, NULL se non compare
 in tale coda
*/
extern pcb_t *out_blocked(pcb_t *p);

/**
 @brief Restituisce senza rimuovere il puntatore al PCB in testa alla coda del
 SEMD associato alla chiave semAdd.
 @param semAdd chiave del semaforo
 @return puntatore al PCB in testa al SEMD, NULL se il SEMD non compare nella
 ASL o se la sua coda è vuota
*/
extern pcb_t *head_blocked(int *semAdd);

#endif
