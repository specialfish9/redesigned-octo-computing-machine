#ifndef TERM_UTILS_H
#define TERM_UTILS_H

/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
extern void print(const char *strp);

/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
extern void print_err(const char *strp);

#endif
