#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <umps3/umps/const.h>

/* todo mettere static */
/** Semaforo Interval Timer */
extern int sem_it;
/** Semaforo disk device */
extern int sem_disk[DEVPERINT];
/** Semaforo flash device */
extern int sem_flash[DEVPERINT];
/** Semaforo net TODO device*/
extern int sem_net[DEVPERINT];
/** Semaforo printer device */
extern int sem_printer[DEVPERINT];
/** Semaforo terminal device (input)*/
extern int sem_term_in[DEVPERINT];
/** Semaforo terminal device (output)*/
extern int sem_term_out[DEVPERINT];

/**
 * @brief Inizializza i semafori dei device.
 */
extern void init_dev_sem(void);

/**
 * @brief Gestisce l'interrupt tirato nella linea specificata.
 * @param line Linea su cui viene tirato l'interrupt
 */
extern void handle_interrupts(const int line);

/**
 * @brief Ricava un puntatore al semaforo del device in base all'indice
 * @param 0-7: Dischi ; 8-15: Flash ; 16-23: Rete ; 24-31: Stampanti ; 32-39:
 * Terminal IN ; 40-47: Terminal OUT; 48: Timer
 *
 * @return int* al valore del semaforo del device richiesto
 */
extern int *get_dev_sem(int);

#endif
