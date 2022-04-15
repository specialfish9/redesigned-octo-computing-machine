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

#endif
