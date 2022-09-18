/**
 *
 * @file init_proc.c
 * @brief Modulo con le funzioni per la creazione dei processi utente di test.
 *
 */
#include "init_proc.h"
#include "support.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "sys_support.h"
#include "syscalls.h"
#include "utils.h"
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


inline void instantiator_proc(void)
{
  state_t tp_states[UPROCMAX];
  static support_t tp_supps[UPROCMAX];
  size_tt i;

  init_supp_structures();

  for (i = 1; i <= UPROCMAX; i++) {
    logi(LOG, "creating uproc", i);

    /* Create state_t and support_t structures for test processes */

    /* state */
    tp_states[i - 1].pc_epc = UPROCSTARTADDR;
    tp_states[i - 1].reg_t9 = UPROCSTARTADDR;
    tp_states[i - 1].reg_sp = USERSTACKTOP;

    /* Timer enabled, interrupts enabled and usermode */
    tp_states[i - 1].status =
        STATUS_TE | STATUS_IEc | STATUS_KUp | STATUS_IM_MASK;

    tp_states[i - 1].entry_hi = i << ENTRYHI_ASID_BIT;

    /* support */
    tp_supps[i - 1].sup_asid = i;
    init_page_table(tp_supps[i - 1].sup_privatePgTbl, i);

    tp_supps[i - 1].sup_exceptContext[0].pc = (memaddr)tlb_exc_handler;
    tp_supps[i - 1].sup_exceptContext[1].pc = (memaddr)support_exec_handler;

    /* Timer enabled, interupts on and kernel mode */
    tp_supps[i - 1].sup_exceptContext[0].status =
        (STATUS_TE | STATUS_IM_MASK | STATUS_KUp | STATUS_IEc) ^ STATUS_KUp;
    tp_supps[i - 1].sup_exceptContext[1].status =
        (STATUS_TE | STATUS_IM_MASK | STATUS_KUp | STATUS_IEc) ^ STATUS_KUp;

    /* Set stack ptr to the end of the stack minus 1 */
    tp_supps[i - 1].sup_exceptContext[0].stackPtr =
        (memaddr)&tp_supps[i].sup_stackTLB[500 - 1];
    tp_supps[i - 1].sup_exceptContext[1].stackPtr =
        (memaddr)&tp_supps[i].sup_stackGen[500 - 1];

    SYSCALL(CREATEPROCESS, (unsigned int)&tp_states[i - 1], PROCESS_PRIO_LOW,
            (unsigned int)&tp_supps[i - 1]);
  }

  for (i = 1; i <= UPROCMAX; i++) {
    p_on_master_sem(); 
  }

  SYSCALL(TERMPROCESS, 0, 0, 0);
}

