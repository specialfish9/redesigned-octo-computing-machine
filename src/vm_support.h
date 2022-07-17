/**
 * @file vm_support.h
 * @brief Modulo contenente le funzioni per la gestione della memoria virtuale
 * */
#ifndef VM_SUPPORT
#define VM_SUPPORT

/**
 * @var Semaforo mutex per la Swap Pool
 * */
extern int swp_pl_sem;

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
