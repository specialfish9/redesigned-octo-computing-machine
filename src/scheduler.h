#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "pandos_types.h"
#include "listx.h"

extern pcb_t *act_proc;
extern struct list_head l_queue;
extern struct list_head h_queue;
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

extern void kill_proc(pcb_t*p);
extern void memcpy(void *dest, void *src, size_tt n);

#endif
