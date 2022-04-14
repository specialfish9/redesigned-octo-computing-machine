#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pcb.h"

/**
 * @brief Gestore delle syscall.
*/
extern int handle_syscall(void);

/** TODO doc*/
extern int passup_or_die(size_tt kind);

/** TODO doc*/
extern int get_ind_from_cmd(unsigned int);

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
