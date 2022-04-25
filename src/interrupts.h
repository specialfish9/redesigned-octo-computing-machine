/**
 *
 * @file interrupts.h
 * @brief Espone le funzioni necessarie per la gestione degli interrput e i
 * semafori associati ai vari device.
 *
 */
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "kernel.h"
#include <umps3/umps/const.h>

/**
 * @var Semaforo Interval Timer
 * */
extern int sem_it;

/**
 * @var Semaforo disk device
 * */
extern int sem_disk[DEVPERINT];

/**
 * @var Semaforo flash device
 * */
extern int sem_flash[DEVPERINT];

/**
 * @var Semaforo network device
 * */
extern int sem_net[DEVPERINT];

/**
 * @var Semaforo printer device
 * */
extern int sem_printer[DEVPERINT];

/**
 * @var Semaforo terminal device (input)
 * */
extern int sem_term_in[DEVPERINT];

/**
 * @var Semaforo terminal device (output)
 * */
extern int sem_term_out[DEVPERINT];

/**
 * @brief Inizializza i semafori dei device.
 */
extern void init_dev_sem(void);

/**
 * @brief Gestisce l'interrupt tirato nella linea specificata.
 * @param line Linea su cui viene tirato l'interrupt
 * @return L'azione che l'excepton handler deve fare una volta gestito
 * l'interrupt
 */
extern enum eh_act handle_interrupts(const int line);

#endif
