/**
 *
 * @file syscalls.h
 * @brief Gestore delle syscall.
 *
 * Espone la funzione necessaria per gestire le system call e alcune altre 
 * funzioni. Espone inoltre passeren e verhogen in quanto sono utili anche per
 * la gestione di alcuni interrupt.
 *
 */
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel.h"
#include "pcb.h"

extern pcb_t *yielded_proc;

/**
 * @brief Gestore delle syscall.
 * @return L'azione che l'exception handler deve eseguire.
 * */
extern enum eh_act handle_syscall(void);

/**
 * @brief Gestisce le eccezioni non gestite dagli appositi handler decidendo 
 * se passarle al livello di supporto o uccidere il processo.
 * @param kind Il tipo
 * @return TRUE se il processo attivo deve essere rimesso in stato di ready,
 * @return FALSE altrimenti.
 * */
extern int passup_or_die(size_tt kind);

/**
 * @brief Syscall PASSEREN (NSYS3). Esegue un'operazione P sul semaforo binario.
 * @param semaddr puntatore al semaforo.
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 **/
extern int passeren(int *semaddr);

/**
 * @brief Syscall VERHOGEN (NSYS4). Esegue un'operazione V sul semaforo binario.
 * @param semaddr puntatore al semaforo.
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
extern pcb_t *verhogen(int *semaddr);

#endif
