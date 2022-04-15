/***********************************utils.h************************************
 *
 * Funzioni di Utility .
 *
 ******************************************************************************/
#ifndef UTILS_H
#define UTILS_H

#include "listx.h"

/* Ottieni il nome della variabile come stringa */
#define NAME_OF(name) #name

/* Utility di debug: stampa il nome di ina varibile seguito dal suo valore.*/
#define DBG(name) dbg_var(#name, name)

/* Stampa su terminal0 la stringa passata in input */
extern void print1(const char *);

/* Stampa su terminal0 la stringa passata in input e fa terminare l'esecuzione
 * con un messaggio di PANIC */
extern void print1_err(const char *);

extern void print1_int(const int);
/* Utility per il debug, stampa il nome di una variabile di tipo int, seguita
 * dal suo valore.*/
extern void dbg_var(const char *, const int);

/*Implementazione standard della funzione memcpy della libc.*/
extern void memcpy(void *dest, void *src, size_tt n);

/*TODO*/
extern size_tt list_size(struct list_head *head);
#endif
