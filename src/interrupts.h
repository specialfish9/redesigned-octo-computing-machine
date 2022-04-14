#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <umps3/umps/const.h>

#define PLTINT 1
#define ITINT 2

/**
 * @brief Ricava un puntatore al semaforo del device in base all'indice
 * @param 0-7: Dischi ; 8-15: Flash ; 16-23: Rete ; 24-31: Stampanti ; 32-39:
 * Terminal IN ; 40-47: Terminal OUT; 48: Timer
 *
 * @return int* al valore del semaforo del device richiesto
 */

extern int *get_dev_sem(int);

/* todo mettere static */
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
