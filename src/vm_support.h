/**
 * @file vm_support.h
 * @brief Modulo contenente le funzioni per la gestione della memoria virtuale
 * */
#ifndef VM_SUPPORT
#define VM_SUPPORT

#include <umps3/umps/const.h>

/*TODO doc*/
#define STK_PG 0x3FFFF
#define FIRST_PG_ADDR 0x80000
#define STK_PG_ADDR 0xBFFFF

/**
 * @brief Indicatore dei semafori per i flash device nella matrice dei semafori
 * dei device
 * */
#define FLASH_SEMS IL_FLASH - IL_DISK

/**
 * @brief Indicatore dei semafori per il printer device nella matrice dei
 * semafori dei device
 * */
#define PRINTER_SEMS IL_PRINTER - IL_DISK

/**
 * @brief Indicatore dei semafori per il terminale nella matrice dei semafori
 * dei device
 * */
#define TERMIN_SEMS IL_TERMINAL - IL_DISK
#define TERMOUT_SEMS IL_TERMINAL - IL_DISK + 1

/**
 * @var Semaforo mutex per la Swap Pool
 * */
extern int swp_pl_sem;

/**
 * @var Semafori per l'accesso in mutua esclusione ai device. Usati dal livello
 * supporto. Vengono inizializzati con @ref init_supp_structures.
 * */
extern int dev_sems[DEVINTNUM][DEVPERINT];

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
