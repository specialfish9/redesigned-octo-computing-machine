/**
 *
 * @file init_proc.c
 * @brief Modulo con le funzioni per la creazione dei processi utente di test.
 *
 */
#include "init_proc.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "sys_support.h"
#include "syscalls.h"
#include "utils.h"
#include "vm_support.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

/* Tag per le utility di log */
#define LOG "IP"

/**
 * @brief Gestore delle eccezioni riguardanti il tlb del livello supporto.
 * */
extern void tlb_exc_handler(void);

/**
 * @brief Gestore delle eccezioni del livello supporto.
 * */
extern void support_exec_handler(void);

/* TODO */
static int semaforo_a_cazzo = 0;

/**
 * @brief Inizializza una tabella delle pagine privata
 * @param tbl la tabella da inizializzare
 * @param asid l'ASID del processo proprietario della tabella
 * */
static void init_page_table(pteEntry_t *tbl, const int asid);

inline void instantiator_proc(void)
{
  state_t tp_states[UPROCMAX];
  static support_t tp_supps[UPROCMAX];
  size_tt i;

  init_supp_structures();

  for (i = 1; i <= 1 /*UPROCMAX*/; i++) {
    logi(LOG, "creating uproc", i);

    /* Create state_t and support_t structures for test processes */

    /* state */
    tp_states[i - 1].pc_epc = UPROCSTARTADDR;
    tp_states[i - 1].reg_t9 = UPROCSTARTADDR;
    tp_states[i - 1].reg_sp = USERSTACKTOP;

    /* Timer enabled, interrupts enabled and usermode */
    tp_states[i - 1].status =
        STATUS_TE | STATUS_IEc | STATUS_KUc | STATUS_IM_MASK;
    /*for (int j = 0; j < 8; j ++) {
      tp_states[i].status |= STATUS_IM_BIT(j);
    }*/
    tp_states[i - 1].entry_hi = 0 & (i << ENTRYHI_ASID_BIT);

    /* support */
    tp_supps[i - 1].sup_asid = i;
    init_page_table(tp_supps[i - 1].sup_privatePgTbl, i);

    context_t context[2];
    context[0].pc = (memaddr)tlb_exc_handler;
    context[1].pc = (memaddr)support_exec_handler;
    /* Timer enabled, interupts on and kernel mode */
    context[0].status = STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEc;
    context[1].status = STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEc;
    /* Set stack ptr to the end of the stack minus 1 */
    context[0].stackPtr = (memaddr)&tp_supps[i].sup_stackTLB[500 - 1];
    context[1].stackPtr = (memaddr)&tp_supps[i].sup_stackGen[500 - 1];

    memcpy(tp_supps[i - 1].sup_exceptContext, context, sizeof(context_t));

    SYSCALL(CREATEPROCESS, (unsigned int)&tp_states[i - 1], PROCESS_PRIO_LOW,
            (unsigned int)&tp_supps[i - 1]);
    logi(LOG, "created uproc ", i);
  }

  for (i = 0; i < 1 /*UPROCMAX*/; i++)
    SYSCALL(PASSEREN, (unsigned int)&semaforo_a_cazzo, 0, 0);
}

inline void init_page_table(pteEntry_t *tbl, const int asid)
{
  size_tt i = 0;

  for (i = 0; i < USERPGTBLSIZE; i++) {
    if (i < USERPGTBLSIZE - 1) {
      /* Se non è l'ultima entry impostiamo il virtual page number */
      tbl[i].pte_entryHI = (0x80000) << VPNSHIFT;
    } else {
      /* Se è l'ultima entry (quella associata alla pagina contenente la stack
       * del uproc) settiamo l'indirizzo 0xBFFFFF000, ovvero il bottom della
       * stack */
      tbl[i].pte_entryHI = 0xBFFFF << VPNSHIFT;
    }

    tbl[i].pte_entryHI |= (asid << ASIDSHIFT);
    tbl[i].pte_entryLO = ENTRYLO_DIRTY | ENTRYLO_GLOBAL;
  }
}
