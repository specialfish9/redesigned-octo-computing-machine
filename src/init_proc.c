#include "init_proc.h"
#include "pandos_types.h"
#include "pandos_const.h"
#include <umps3/umps/cp0.h>
#include "utils.h"
#include "syscalls.h"

#define TEST_PROCS 8

static pteEntry_t *page_tables[MAXPROC];

static void init_support_structures(void);

// TODO tmp
static void tlb_refill_handler(void);
// TODO tmp
static void exc_handler(void);

inline void instantiator_proc(void) {
  state_t tp_states[TEST_PROCS];
  support_t tp_supps[TEST_PROCS];
  size_tt i;
  size_tt stack_gen_counter = 499;


  init_support_structures();


  for (i = 0; i < TEST_PROCS; i++) {
    /* Create state_t and support_t structures for test processes */

    /* state */
    tp_states[i].pc_epc = UPROCSTARTADDR;
    tp_states[i].reg_t9 = UPROCSTARTADDR;
    tp_states[i].reg_sp = USERSTACKTOP;
    /* Timer enabled, interrupts enabled and usermode */ 
    tp_states[i].status = ALLOFF | STATUS_TE | STATUS_IM_MASK | STATUS_IEp;
    tp_states[i].entry_hi = 0 & (i << ENTRYHI_ASID_BIT);

    /* support */
    tp_supps[i].sup_asid = i;
    page_tables[i] = tp_supps[i].sup_privatePgTbl;

    context_t context[2];
    context[0].pc = (memaddr) tlb_refill_handler;
    context[1].pc = (memaddr) exc_handler;
    context[0].status = ALLOFF | STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEp;
    context[1].status = ALLOFF | STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEp;
    context[0].stackPtr = tp_supps[i].sup_stackGen[stack_gen_counter--];
    context[1].stackPtr = tp_supps[i].sup_stackTLB[stack_gen_counter--];
    memcpy(tp_supps[i].sup_exceptState, context, sizeof(context_t));

    
    create_process(&tp_states[i], PROCESS_PRIO_HIGH, &tp_supps[i]);
  }

}

static void init_support_structures(void) {
  /* TODO init swap pool */
  /* TODO init shared device sems */
  
}

static void tlb_refill_handler(void) {}
// TODO tmp
static void exc_handler(void){}

