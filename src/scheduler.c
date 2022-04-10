#include "scheduler.h"
#include "klog.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "utils.h"
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOG(s) print1("[Scheduler]" s)

pcb_t *act_proc;

static size_tt procs_count;
static size_tt sb_procs;
static struct list_head l_queue;
static struct list_head h_queue;
static unsigned int pid_count = 1;

inline void init_scheduler(void)
{
  procs_count = 0;
  sb_procs = 0;
  act_proc = NULL;
  mk_empty_proc_q(&l_queue);
  mk_empty_proc_q(&h_queue);
}

inline void create_init_proc(const memaddr entry_point)
{
  pcb_t *proc;

  if ((proc = alloc_pcb()) == NULL) {
    kprint("Impossible to allocate init process PCB");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = entry_point;
  proc->p_s.status |= STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEc;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = 1; // TODO

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

inline void scheduler_next(void)
{

  LOG("Chosing next process...\n");

  if (empty_proc_q(&h_queue) == FALSE) {
    /* Scegli un processo a priorità alta */
    act_proc = remove_proc_q(&h_queue);
    if (&(act_proc->p_s) == NULL) {
      LOG("Something wrong with high prior queue. Panicing...\n");
      PANIC();
    }
    LOG("Loading high priority process with PID ");
    print1_int(act_proc->p_pid);
    /* Aggiorno l'age del processo */
    STCK(act_proc->p_time);
    /* Lo carico */
    LDST(&(act_proc->p_s));
  } else if (empty_proc_q(&l_queue) == FALSE) {
    /* Scegli un processo a priorità bassa */
    act_proc = remove_proc_q(&l_queue);
    LOG("Loading low priority process with PID ");
    print1_int(act_proc->p_pid);
    setTIMER(TIMESLICE);
    /* Aggiorno l'age del processo */
    STCK(act_proc->p_time);
    /* Lo carico */
    LDST(&act_proc->p_s);

  } else if (!procs_count) {
    print1_err("No process alive: halting...");
    HALT();
  } else if (procs_count && sb_procs) {
    /* TODO set status register to ebable interrupts and
     * disable the plt */
    print1_err("No process available: waiting...");
    WAIT();
  } else if (procs_count && !sb_procs) {
    /* DEADLOCK !*/
    print1_err("DEADLOCK: panicing...");
    PANIC();
  }
}

inline pcb_t *mk_proc(state_t *statep, int prio, support_t *supportp)
{
  pcb_t *result;

  result = alloc_pcb();

  if (result == NULL) {
    return NULL;
  }

  result->p_supportStruct = supportp;
  result->p_prio = prio;
  memcpy(&result->p_s, statep, sizeof(state_t)); 
  result->p_pid = (unsigned int) result;
  ++pid_count;

  if (prio == PROCESS_PRIO_HIGH) {
    insert_proc_q(&h_queue, result);
  } else if (prio == PROCESS_PRIO_LOW) {
    insert_proc_q(&l_queue, result);
  }
  insert_child(act_proc, result);
  procs_count++;

  return result;
}

inline void kill_proc(pcb_t *p)
{
  procs_count--;
  if (p->p_prio == PROCESS_PRIO_HIGH) {
    out_proc_q(&h_queue, p);
  } else if (p->p_prio == PROCESS_PRIO_LOW) {
    out_proc_q(&l_queue, p);
  }
  free_pcb(p);
}

inline void enqueue_proc(pcb_t *const pcb, const unsigned int priority)
{
  if (priority == PROCESS_PRIO_HIGH) {
    insert_proc_q(&h_queue, pcb);
  } else if (priority == PROCESS_PRIO_LOW) {
    insert_proc_q(&l_queue, pcb);
  }
}

inline pcb_t* dequeue_proc(const unsigned int priority) {
  if (priority == PROCESS_PRIO_HIGH) {
    return remove_proc_q(&h_queue);
  } else if (priority == PROCESS_PRIO_LOW) {
    return remove_proc_q(&l_queue);
  }
  return NULL;
}

inline pcb_t* rm_proc(pcb_t* const pcb, const unsigned int priority)
{
  if (priority == PROCESS_PRIO_HIGH) {
    return out_proc_q(&h_queue, pcb);
  } else if (priority == PROCESS_PRIO_LOW) {
    return out_proc_q(&l_queue, pcb);
  }
  return NULL;
  
}

