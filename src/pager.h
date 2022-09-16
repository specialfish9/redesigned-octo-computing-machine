/**
 * @file pager.h
 * @brief Modulo contenente le funzioni per la gestione della memoria virtuale
 * */
#ifndef PAGER
#define PAGER

#include <umps3/umps/const.h>

/**
 * @const VPN della pagina contenente lo stack
 * */
#define STK_PG 0x3FFFF

/**
 * @brief Indicatore dei semafori per i flash device nella matrice dei semafori
 * dei device
 * */
#define FLASH_SEMS IL_FLASH - IL_DISK

/**
 * @brief Indicatore dei semafori per il printer device nella matrice dei
 * semafori dei device
 * */
#define PRINTER_SEMS (IL_PRINTER - IL_DISK - 1)

/**
 * @brief Indicatore dei semafori per il terminale nella matrice dei semafori
 * dei device
 * */
#define TERMIN_SEMS (IL_TERMINAL - IL_DISK - 1)
#define TERMOUT_SEMS (IL_TERMINAL - IL_DISK)

/**
 * @var Semaforo mutex per la Swap Pool
 * */
extern int swp_pl_sem;

/**
 * @var Semafori per l'accesso in mutua esclusione ai device. Usati dal livello
 * supporto. Vengono inizializzati con @ref init_supp_structures.
 * */
extern int dev_sems[DEVINTNUM + 1][DEVPERINT];

/**
 * @brief Inizializza tutte le strutture dati necessarie per la gestione
 * della memoria virtuale.
 * */
extern void init_supp_structures(void);

/**
 * @brief Gestore delle eccezioni del TLB. The pager
 * */
extern void tlb_exc_handler(void);

#endif
