#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "listx.h"
#include "pcb.h"

int handle_syscall(void);
int passup_or_die(size_tt kind);

void passeren(int *semaddr);
pcb_t *verhogen(int *semaddr);

#endif
