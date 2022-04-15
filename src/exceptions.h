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
  @brief Trova l'indice che identifica il device a partire dall'indirizzo del suo 
  command register Se non si usa esternamente posso non metterla nel .h giusto?
  @param cmd_addr Indirizzo del command register.
  @return L'indice identificativo del device. -1 se non lo trova.
*/
extern int get_ind_from_cmd(unsigned int cmd_addr);

/**
  Esegue un'operazione P sul semaforo binario.
  @param semaddr puntatore al semaforo.
*/
extern void passeren(int *semaddr);

/**
  Esegue un'operazione V sul semaforo binario.
  @param semaddr puntatore al semaforo.
  @return todo
*/
extern pcb_t *verhogen(int *semaddr);

#endif
