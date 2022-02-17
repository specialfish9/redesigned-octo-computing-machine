/******************************term_utils.h************************************
 *
 * Utility per stampare nel terminale.
 *
 ******************************************************************************/
#ifndef TERM_UTILS_H
#define TERM_UTILS_H

/* Ottieni il nome della variabile come stringa */
#define NAME_OF(name) #name

#define DBG(name) dbg_var(#name, name)

/* Stampa su terminal0 la stringa passata in input */
extern void print(const char *);

/* Stampa su terminal0 la stringa passata in input e fa terminare l'esecuzione
 * con un messaggio di PANIC */
extern void print_err(const char *);

/* Utility per il debug, stampa il nome di una variabile di tipo int, seguita
* dal suo valore.*/
extern void dbg_var(const char*, const int);

#endif
