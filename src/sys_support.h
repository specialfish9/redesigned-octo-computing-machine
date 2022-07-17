#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pandos_types.h"

/**
 * @brief Handler generale per il livello supporto. Gestisce la divisione tra
 * syscall e trap
 */
extern void support_exec_handler(void);

/**
 * @brief Termina il processo dopo aver rilasciato un eventuale semaforo su 
 * cui era fermo
 * */
extern void safe_kill(void);

#endif
