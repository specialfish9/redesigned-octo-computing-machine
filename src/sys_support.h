#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

extern unsigned int get_TOD(void);
extern void terminate(void);
extern int write_to_printer(unsigned int virtAddr, int len, unsigned int asid);
extern int write_to_terminal(unsigned int virtAddr, int len, unsigned int asid);
extern int read_from_terminal(unsigned int virtAddr);


#endif

