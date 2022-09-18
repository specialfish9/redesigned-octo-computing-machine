/**
 * @file sys_support.h
 * @brief Gestore delle syscall del livello supporto.
 *
 * Espone la funzione necessaria per gestire le system call e alcune altre
 * funzioni. 
 *
 */
#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pandos_types.h"
/**
 * @brief Gestore delle systemcall a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
extern void support_syscall_handler(support_t *act_proc_sup);

/**
 * @brief Termina il processo dopo aver rilasciato un eventuale semaforo su
 * cui era fermo
 * */
extern void safe_kill(void);

#endif
