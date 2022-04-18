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

/* Utility per il debug, stampa il nome di una variabile di tipo int, seguita
 * dal suo valore.*/
extern void dbg_var(const char *, const int);

/*Implementazione standard della funzione memcpy della libc.*/
extern void memcpy(void *dest, void *src, size_tt n);

extern void log(char* tag, char* mex);
extern void logi(char* tag, char* mex, int value);

#endif
