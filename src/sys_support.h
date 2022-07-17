#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

#include "pandos_types.h"

/**
 * @brief Handler generale per il livello supporto. Gestisce la divisione tra
 * syscall e trap
 */
extern void support_exec_handler(void);

/* TODO desc */
extern void safe_kill(void);

#endif
