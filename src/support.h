/**
 * @file support.h
 * @brief Modulo contenente le funzioni utili per la gestione del livello supporto
 * */
#ifndef SUPPORT_H
#define SUPPORT_H

#include "pandos_types.h"
#include <umps3/umps/const.h>

/**
 * @brief Indicatore dei semafori per i flash device nella matrice dei semafori
 * dei device
 * */
#define FLASH_SEMS IL_FLASH - IL_DISK

/**
 * @brief Indicatore dei semafori per il printer device nella matrice dei
 * semafori dei device
 * */
#define PRINTER_SEMS (IL_PRINTER - IL_DISK)

/**
 * @brief Indicatore dei semafori per il terminale nella matrice dei semafori
 * dei device
 * */
#define TERMIN_SEMS (IL_TERMINAL - IL_DISK)
#define TERMOUT_SEMS (IL_TERMINAL - IL_DISK + 1)

/**
 * @brief Inizializza tutte le strutture dati necessarie per la gestione
 * della memoria virtuale.
 * */
extern void init_supp_structures(void);

/**
 * @brief Inizializza una tabella delle pagine privata
 * @param tbl la tabella da inizializzare
 * @param asid l'ASID del processo proprietario della tabella
 * */
extern void init_page_table(pteEntry_t *tbl, const int asid);

/**
 * @brief Handler generale per il livello supporto. Gestisce la divisione tra
 * syscall e trap
 */
extern void support_exec_handler(void);

/**
 * @brief Utility per compiere una P sul semaforo di un device
 * @param type Il tipo di device appartenente all'intervallo [0, 5]
 * @param no Numero del device appartenente all'intervallo [0, 7]
 * */
extern void p_on_dev(const unsigned int type, const unsigned int no);

/**
 * @brief Utility per compiere una V sul semaforo di un device
 * @param type Il tipo di device appartenente all'intervallo [0, 5]
 * @param no Numero del device appartenente all'intervallo [0, 7]
 * */
extern void v_on_dev(const unsigned int type, const unsigned int no);

/**
 * @brief Utility per compiere una P sul semaforo master
 * */
extern void p_on_master_sem(void);

/** 
 * @brief Utility per compiere una V sul semaforo master
 * */
extern void v_on_master_sem(void);

#endif

