/**
 *
 * @file utils.h
 * @brief Funzioni di Utility.
 *
 */
#ifndef UTILS_H
#define UTILS_H

#include "listx.h"

/**
 * @brief Macro per ottenere il nome di una variabile come stringa.
 * @param name Il nome della variabile.
 * */
#define NAME_OF(name) #name

/**
 * @brief Macro utility per il debug: stampa il nome di una varibile seguito
 * dal suo valore.
 * @param Il nome della variabile */
#define DBG(name) dbg_var(#name, name)

/**
 * @brief Utility per il debug, stampa il nome di una variabile di tipo int,
 * seguita dal suo valore.
 * @param Il nome della variabile
 * @param Il suo valore
 * */
extern void dbg_var(const char *, const int);

/**
 * @brief Implementazione standard della funzione memcpy della libc.
 * @param dest L'indirizzo di destinazione su cui copiare.
 * @param src L'indirizzo di partenza da cui copiare.
 * @param n Il numero di byte da copiare.
 * */
extern void memcpy(void *dest, void *src, size_tt n);

/**
 * @brief Funzione per loggare un messaggio sul buffer di klog
 * @param tag Log tag
 * @param mex Il messaggio
 * */
extern void log(char *tag, char *mex);

/**
 * @brief Funzione per loggare un messaggio sul buffer di klog seguito da un
 * valore intero.
 * @param tag Log tag.
 * @param mex Il messaggio.
 * @param value Il valore intero.
 * */
extern void logi(char *tag, char *mex, int value);

#endif
