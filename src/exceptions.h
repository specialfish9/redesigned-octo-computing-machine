#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pcb.h"

/**
  @brief Gestore delle syscall.
  @return TRUE se il processo attivo deve essere rimesso in stato di ready,
  FALSE altrimenti.
*/
extern int handle_syscall(void);

/**
  @brief Gestisce le eccezioni non gestite dagli appositi handler decidendo
  se passarle al livello di supporto o uccidere il processo.
  @param kind Il tipo
  @return TRUE se il processo attivo deve essere rimesso in stato di ready,
  FALSE altrimenti.
*/
extern int passup_or_die(size_tt kind);

/**
  @brief Esegue un'operazione P sul semaforo binario.
  @param semaddr puntatore al semaforo.
*/
extern void passeren(int *semaddr);

/**
  @brief Esegue un'operazione V sul semaforo binario.
  @param semaddr puntatore al semaforo.
  @return todo
*/
extern pcb_t *verhogen(int *semaddr);

#endif
