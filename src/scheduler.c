/**
 *
 * @file scheduler.h
 * @brief Implementazioni delle funzioni dello scheduler.
 *
 */
#include "scheduler.h"
#include "asl.h"
#include "pcb.h"
#include "syscalls.h"
#include "utils.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

#define LOG(s) log("S", s)
#define LOGi(s, i) logi("S", s, i)

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
    LOG("Impossible to allocate init process");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = entry_point;
  proc->p_s.status =
      ALLOFF | STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEp;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = (memaddr)proc;

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

inline void scheduler_next(void)
{
  pcb_t *next_proc;

  if (!empty_proc_q(&h_queue)) {
    /* Scegli un processo a priorità alta */
    next_proc = dequeue_proc(PROCESS_PRIO_HIGH);
    if (&(next_proc->p_s) == NULL) {
      LOG("Something wrong with high prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (!empty_proc_q(&l_queue)) {
    /* Scegli un processo a priorità bassa */
    next_proc = dequeue_proc(PROCESS_PRIO_LOW);
    if (&(next_proc->p_s) == NULL) {
      LOG("Something wrong with low prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (!procs_count) {
    LOG("No process alive: halting");
    HALT();
  } else if (procs_count > 0 && sb_procs > 0) {
    if (yielded_proc != NULL) {
      enqueue_proc(yielded_proc, yielded_proc->p_prio);
      yielded_proc = NULL;
    }
    act_proc = NULL;
    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
    /* Abilita gli interrupt e disabilita il timer */
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_TE) ^ STATUS_TE);
    LOG("No ready process available: waiting...");
    WAIT();
    // LOG("wait recieved a masked interrupt");
    scheduler_next();
  } else if (procs_count > 0 && !sb_procs) {
    /* DEADLOCK !*/
    LOG("DEADLOCK: panicing");
    PANIC();
  } else {
    /* Non dovremmo mai finire qua */
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
  result->p_pid = (memaddr)result;

  insert_child(act_proc, result);
  procs_count++;

  enqueue_proc(result, prio);

  return result;
}

inline void kill_proc(pcb_t *p)
{
  if (p == NULL) {
    PANIC();
  }

  --procs_count;
  if (p->p_parent != NULL && out_child(p) != p) {
    PANIC();
  }
  if (p->p_semAdd != NULL) {
    --sb_procs;
    if (out_blocked(p) == NULL) {
      LOGi("Could not remove process from semaphore ", p->p_pid);
      PANIC();
    }
  }
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
  pcb_t *p = NULL;
  if (priority == PROCESS_PRIO_HIGH) {
    p = remove_proc_q(&h_queue);
  } else if (priority == PROCESS_PRIO_LOW) {
    p = remove_proc_q(&l_queue);
  }
  return p;
}

inline void load_proc(pcb_t *pcb)
{
  if (pcb == NULL) {
    LOG("Attempt to load NULL pcb");
    return;
  }
  if (yielded_proc != NULL) {
    enqueue_proc(yielded_proc, yielded_proc->p_prio);
    yielded_proc = NULL;
  }

  act_proc = pcb;

  setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
  act_proc->p_s.status |= STATUS_IEp | STATUS_TE;
  if (act_proc->p_prio == PROCESS_PRIO_LOW) {
  } else if (act_proc->p_prio == PROCESS_PRIO_HIGH) {
    act_proc->p_s.status ^= STATUS_TE;
  }
  /* Aggiorno l'ultimo update dell'age del processo */
  STCK(act_proc->p_tm_updt);

  /* Lo carico */
  LDST(&act_proc->p_s);
}
