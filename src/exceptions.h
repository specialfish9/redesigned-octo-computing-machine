#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pcb.h"

extern int handle_syscall(void);
extern int passup_or_die(size_tt kind);
extern int get_ind_from_cmd(unsigned int);
/**
  Esegue un'operazione P sul semaforo binario.
  @param semaddr: puntatore al semaforo.
 */
extern void passeren(int *semaddr);

/**
  Esegue un'operazione V sul semaforo binario.
  @param semaddr: puntatore al semaforo.
 */
extern pcb_t *verhogen(int *semaddr);

#endif
