#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define ITINT 2
#define DEV_SEM_LEN 7

/* linea int -> semaforo */
extern int dev_sem[DEV_SEM_LEN]; /* TODO probabilmente serve più grande*/

extern void init_dev_sem(void);

extern void handle_interrupts(const int line);

#endif
