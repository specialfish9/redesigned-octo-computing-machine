#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <umps3/umps/const.h>

#define PLTINT 1
#define ITINT 2

/* linea int -> semaforo */
extern int sem_it;
extern int sem_disk[DEVPERINT];
extern int sem_flash[DEVPERINT];
extern int sem_net[DEVPERINT];
extern int sem_printer[DEVPERINT];
extern int sem_term_in[DEVPERINT];
extern int sem_term_out[DEVPERINT];

extern void init_dev_sem(void);

extern void handle_interrupts(const int line);

#endif
