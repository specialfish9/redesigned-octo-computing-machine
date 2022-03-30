#include "scheduler.h"
#include "listx.h"
#include "pcb.h"
#include "term_utils.h"
#include "pandos_const.h"
#include <umps3/umps/libumps.h>

#define LOG(s) print1("[Schedulah]" s)

void scheduler_next(pcb_t *current, const size_tt proc_count,
                    const size_tt sb_count, struct list_head *h_queue,
                    struct list_head *l_queue)
{
  
  LOG("Chosing next process...\n");

  if (empty_proc_q(h_queue) == FALSE) {
    /* Scegli un processo a priorità alta */
    current = remove_proc_q(h_queue);
    LOG("Loading high priority process with PID ");
    print1_int(current->p_pid);
    LDST(&current->p_s);

  } else if (empty_proc_q(l_queue)) {
    /* Scegli un processo a priorità bassa */
    current = remove_proc_q(l_queue);
    LOG("Loading low priority process with PID ");
    print1_int(current->p_pid);
    setTIMER(TIMESLICE);
    LDST(&current->p_s);

  } else if (!proc_count) {
    print1_err("No process alive: halting...");
    HALT();
  } else if (proc_count && sb_count) {
    /* TODO set status register to ebable interrupts and
     * disable the plt */
    print1_err("No process available: waiting...");
    WAIT();
  } else if (proc_count && !sb_count) {
    /* DEADLOCK !*/
    print1_err("DEADLOCK: panicing...");
    PANIC();
  }
}
