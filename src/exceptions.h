#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "pandos_types.h"

/**
  Crea un nuovo processo come figlio del chiamante.
  @param statep: stato che deve avere il processo.
  @param prio: priorità da assegnare al processo.
  @param supportp: puntatore alla struttura supporto del processo.
  @return Il PID de processo se la syscall ha successo -1 altrimenti
 */
extern int create_process(state_t *statep, int prio, support_t *suppportp);

// extern void termniate_process(int pid);

extern void passeren(int *semaddr);

extern void verhogen(int *semaddr);

// extern int do_io(int *cmd_addr, int cmd_val);

// extern int get_cpu_time(void);

// extern int wait_for_clock(void);

// extern support_t* get_support_data(void);

// extern int get_proc_id(int parent);

// extern int yield(void);

#endif
