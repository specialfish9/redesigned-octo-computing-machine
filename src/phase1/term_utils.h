#ifndef TERM_UTILS_H
#define TERM_UTILS_H

#include "pandos_const.h"
#include "pandos_types.h"

/* #include "asl.h" */
#include <umps/libumps.h>

#define TRANSMITTED 5
#define ACK 1
#define PRINTCHR 2
#define CHAROFFSET 8
#define STATUSMASK 0xFF
#define TERM0ADDR 0x10000254

typedef unsigned int devreg;

/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void print(char *strp);

/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void print_err(char *strp);
#endif
