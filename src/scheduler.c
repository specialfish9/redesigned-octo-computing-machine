#include "scheduler.h"
#include "listx.h"
#include "pcb.h"
#include "term_utils.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>

void scheduler_next(pcb_t *current, const size_tt proc_count,
                    const size_tt sb_count, struct list_head *h_queue,
                    struct list_head *l_queue)
{
  pcb_t *first;

  if (empty_proc_q(h_queue) == FALSE) {
    /* Scegli un processo a priorità alta */
    first = remove_proc_q(h_queue);
    current = first; /* TODO dagling pointer ?*/
    LDST(&current->p_s);
  } else if (empty_proc_q(l_queue)) {
    /* Scegli un processo a priorità bassa */
    first = remove_proc_q(l_queue);
    /* TODO load 5 millisec on the plt */
    current = first; /* TODO dagling pointer ?*/
    LDST(&current->p_s);
  } else if (!proc_count) {
    HALT();
  } else if (proc_count && sb_count) {
    /* TODO set status register to ebable interrupts and
     * disable the plt */
    WAIT();
  } else if (proc_count && !sb_count) {
    /* DEADLOCK !*/
    PANIC();
  }
}
