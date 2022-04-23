/**
 *
 * @file klog.h
 * @author Maldus512
 * @brief Small library that implements a circular log buffer. When properly
 * traced (with ASCII representation), `klog_buffer` displays a series of
 * printed lines.
 *
 */
#ifndef KLOG_H
#define KLOG_H

/**
 * @brief Stampa la stringa passata nel buffer in memoria
 * @param La stringa da stampare
 * */
void kprint(char *);
/**
 * @brief Stampa il valore esadecimale passato come parametro nel buffer di 
 * memoria.
 * @param Il valore esadecimale.
 * */
void kprint_hex(unsigned int);
/**
 * @brief Stampa il valore di un intero passato come parametro nel buffer di 
 * memoria.
 * @param L'intero da stampare.
 * */
void kprint_int(int);

#endif
