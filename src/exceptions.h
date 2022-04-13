#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "listx.h"
#include "pcb.h"

extern int handle_syscall(void);
extern int passup_or_die(size_tt kind);
extern int get_ind_from_cmd(unsigned int);

#endif
