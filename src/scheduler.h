#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pandos_types.h"

extern void init_scheduler(void);

extern void create_init_proc(const memaddr entry_point);

extern void scheduler_next(void);

/**
 * Crea un processo figlio del processo correntemente in esecuzione.
 * @param statep: puntatore allo stato del nuovo processo.
 * @param prio: priorita' del nuovo processo.
 * @param supportp: puntatore alla struttura di supporto del nuovo processo.
 * @return il pcb del processo creato o NULL se non e' possibile crearlo */
extern pcb_t *mk_proc(state_t *statep, int prio, support_t *supportp);

#endif
