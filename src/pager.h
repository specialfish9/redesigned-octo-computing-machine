/**
 * @file pager.h
 * @brief Modulo contenente le funzioni per la gestione della memoria virtuale
 * */
#ifndef PAGER_H
#define PAGER_H

#include <umps3/umps/const.h>
#include "pandos_types.h"

/**
 * @brief VPN della pagina contenente lo stack
 * */
#define STK_PG 0x3FFFF

/**
 * @brief Dimensione della Swap Pool
 * */
#define SP_SIZE (2 * UPROCMAX)

/**
 * @struct sp_entry_t
 * @brief Rappresenta una entry della swap table.
 * @var asid Indica l'ASID del processo propretario della pagina salvata
 * nella entry
 * @var vpn Indica il numero della pagina salvata nella entry
 * @ pg_tbl_entry Puntatore alla entry dentro la tabella delle pagine privata
 * del processo
 * */
typedef struct {
  int asid;
  int vpn;
  pteEntry_t *pg_tbl_entry;
} sp_entry_t;

/**
 * @var Semaforo mutex per la swap pool
 * */
extern int sp_sem;

/**
 * @var Array con le entry della swap pool
 * */
extern sp_entry_t sp_tbl[SP_SIZE];

/**
 * @var Variabile per tenere conto dei processi che occupano la swap pool e sono
 * vivi. In particolare essendo una variabile di tipo char e' composta da 8 bit.
 * Se il bit i-esimo e' a 1 significa che la swap pool contiene frame del processo
 * con asid i + 1 e che questo processo e' vivo. Se il bit i-esimo e' 0 allora
 * il processo e' morto e/o la swap pool non contiene frame di sua proprieta'.
 * */
extern char sp_asids;

/**
 * @brief Gestore delle eccezioni del TLB. The pager
 * */
extern void tlb_exc_handler(void);

/**
 * @brief Utilizza sp_asids per marcare i frame del processo specificato come 
 * utilizzabili
 * @param asid L'asid del processo coinvolto
 * */
extern void clean_frames(const unsigned int asid);

#endif
