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

#define LOG(s) kprint("S>" s "|")
#define LOGi(s, i)                                                             \
  kprint("S>" s);                                                              \
  kprint_int(i);                                                               \
  kprint("|")

/** Processo attivo */
pcb_t *act_proc;
/** Contatore processi soft blocked */
size_tt sb_procs;
/** Numero di processi */
static size_tt procs_count;
/** Coda a bassa priorità */
static struct list_head l_queue;
/** Coda ad alta priorità */
static struct list_head h_queue;

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
    LOG("Imp all init");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = entry_point;
  proc->p_s.status |= STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEc;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = (memaddr) proc; 

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

inline void scheduler_next(void)
{
  pcb_t *next_proc;

  LOGi("procs_count", procs_count);
  LOGi("sb_count", sb_procs);
  LOGi("HPLEN", list_size(&h_queue));
  LOGi("LPLEN", list_size(&l_queue));

  LOG("ch proc");

  if (empty_proc_q(&h_queue) == FALSE) {
    /* Scegli un processo a priorità alta */
    next_proc = remove_proc_q(&h_queue);
    if (&(next_proc->p_s) == NULL) {
      LOG("Something wrong with high prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (empty_proc_q(&l_queue) == FALSE) {
    /* Scegli un processo a priorità bassa */
    next_proc = remove_proc_q(&l_queue);
    if (&(next_proc->p_s) == NULL) {
      LOG("Something wrong with low prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (!procs_count) {
    LOG("No process alive: halting");
    HALT();
  } else if (procs_count > 0 && sb_procs > 0) {
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_TE) ^ STATUS_TE);
    LOG("No process available: waiting");
    WAIT();
  } else if (procs_count > 0&& !sb_procs) {
    /* DEADLOCK !*/
    LOG("DEADLOCK: panicing");
    PANIC();
  }
  
  LOG("panic at the disco");
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
  result->p_pid = (unsigned int)result;

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

inline pcb_t *dequeue_proc(const unsigned int priority)
{
  if (priority == PROCESS_PRIO_HIGH) {
    return remove_proc_q(&h_queue);
  } else if (priority == PROCESS_PRIO_LOW) {
    return remove_proc_q(&l_queue);
  }
  return NULL;
}

inline pcb_t *rm_proc(pcb_t *const pcb, const unsigned int priority)
{
  if (priority == PROCESS_PRIO_HIGH) {
    return out_proc_q(&h_queue, pcb);
  } else if (priority == PROCESS_PRIO_LOW) {
    return out_proc_q(&l_queue, pcb);
  }
  return NULL;
}

inline void load_proc(pcb_t *pcb){
  if (pcb == NULL) {
    LOG("Attempt to load NULL pcb");
    return;
  }
  
  act_proc = pcb;

  if (act_proc->p_prio == PROCESS_PRIO_LOW) {
    LOGi("Load lp pro", act_proc->p_pid);

    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));

  } else if(act_proc->p_prio == PROCESS_PRIO_HIGH) {
    LOGi("Load hp pro", act_proc->p_pid);
  } 
    /* Aggiorno l'age del processo */
    STCK(act_proc->p_tm_updt);

    /* Lo carico */
    LDST(&act_proc->p_s);
}
