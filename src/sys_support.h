#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H
#include "pandos_types.h"

extern unsigned int get_TOD(void);
extern void terminate(void);
extern int write_to_printer(unsigned int virtAddr, int len, unsigned int asid);
extern int write_to_terminal(unsigned int virtAddr, int len, unsigned int asid);
extern void support_trap_handler(support_t* act_proc_sup);
extern int read_from_terminal(unsigned int virtAddr, unsigned int asid);


#endif

