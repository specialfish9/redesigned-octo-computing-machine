/******************************term_utils.h************************************
 *
 * Utility per stampare nel terminale.
 *
 ******************************************************************************/
#ifndef TERM_UTILS_H
#define TERM_UTILS_H

/* Stampa su terminal0 la stringa passata in input */
extern void print(const char *);

/* Stampa su terminal0 la stringa passata in input e fa terminare l'esecuzione
 * con un messaggio di PANIC */
extern void print_err(const char *);

#endif
