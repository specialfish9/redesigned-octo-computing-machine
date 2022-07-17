#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H
#include "pandos_types.h"

#include "pandos_types.h"

/**
 * @brief Gestore per le trap a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
extern void support_trap_handler(support_t *act_proc_sup);

/**
 * @brief Handler generale per il livello supporto. Gestisce la divisione tra
 * syscall e trap
 */
extern void support_exec_handler(void);

#endif
