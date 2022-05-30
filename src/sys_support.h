#ifndef SYSSUPPORT_H
#define SYSSUPPORT_H

extern unsigned int get_TOD(void);
extern void terminate(void);
extern int write_to_printer(char *virtAddr, int len);
extern int write_to_terminal(char *virtAddr, int len);
extern int read_from_terminal(char *virtAddr);


#endif

