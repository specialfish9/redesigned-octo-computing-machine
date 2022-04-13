#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#define ITINT 2
#define DEV_SEM_LEN 7

/**
 * @brief Ricava un puntatore al semaforo del device in base all'indice
 * @param 0-7: Dischi ; 8-15: Flash ; 16-23: Rete ; 24-31: Stampanti ; 32-39: Terminal IN ; 40-47: Terminal OUT; 48: Timer
 * 
 * @return int* al valore del semaforo del device richiesto
 */

extern int *get_dev_sem(int);

extern void init_dev_sem(void);

extern void handle_interrupts(const int line);

#endif
