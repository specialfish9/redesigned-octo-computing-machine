#include "scheduler.h"
#include "asl.h"
#include "exceptions.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "utils.h"
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOG(s) kprint("S>" s "\n")
#define LOGi(s, i)                                                             \
  kprint("S>" s);                                                              \
  kprint_int(i);                                                               \
  kprint("\n")

static int pidc = 1; // TODO leva

/** Processo attivo */
pcb_t *act_proc;
/** Contatore processi soft blocked */
size_tt sb_procs;
/** Numero di processi */
static size_tt procs_count;
static size_tt l_sz;
static size_tt h_sz;
/** Coda a bassa priorità */
struct list_head l_queue;
/** Coda ad alta priorità */
struct list_head h_queue;

inline void init_scheduler(void)
{
  procs_count = 0;
  sb_procs = 0;
  act_proc = NULL;
  l_sz = 0;
  h_sz = 0;
  mk_empty_proc_q(&l_queue);
  mk_empty_proc_q(&h_queue);
}

inline void create_init_proc(const memaddr entry_point)
{
  pcb_t *proc;

  if ((proc = alloc_pcb()) == NULL) {
    LOG("!!! Imp all init");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = entry_point;
  proc->p_s.status |=
      0 | IEPON | IMON | TEBITON; // | STATUS_TE | STATUS_IM_MASK | STATUS_KUc |
                                  // STATUS_IEc | STATUS_IEp;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = pidc++; //(memaddr)proc;

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

static void log_status(void)
{
  kprint("process_count ");
  kprint_int(procs_count);
  kprint("\n");
  kprint("sb_count ");
  kprint_int(sb_procs);
  kprint("\n");
  // kprint("hs");
  // kprint_int(h_sz);
  // kprint("ls");
  // kprint_int(l_sz);
  // kprint(")");
}

inline static void print_queue(const char *prefix, struct list_head *h)
{
  kprint("S>[");
  struct list_head *ptr;
  int i = 0;
  list_for_each(ptr, h)
  {
    pcb_t *pcb = container_of(ptr, pcb_t, p_list);
    kprint_int((unsigned int)pcb);
    kprint((char *)prefix);
    kprint(",");
    i++;
  }
  kprint("]");
}

inline void scheduler_next(void)
{
  pcb_t *next_proc;

  log_status();

  if (empty_proc_q(&h_queue) == FALSE) {
    /* Scegli un processo a priorità alta */
    next_proc = dequeue_proc(PROCESS_PRIO_HIGH);
    if (&(next_proc->p_s) == NULL) {
      kprint("!!! Something wrong with high prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (empty_proc_q(&l_queue) == FALSE) {
    /* Scegli un processo a priorità bassa */
    next_proc = dequeue_proc(PROCESS_PRIO_LOW);
    if (&(next_proc->p_s) == NULL) {
      kprint("!!! Something wrong with low prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (!procs_count) {
    LOG("No process alive: halting");
    HALT();
  } else if (procs_count > 0 && sb_procs > 0) {
    /* buona cosa da avere, evita che vengano fatte cazzate nella gestione degli
     * interrupt dopo una wait */
    LOG("npa:wait");
    if (yielded_process != NULL) {
      enqueue_proc(yielded_process, yielded_process->p_prio);
      yielded_process = NULL;
    }
    act_proc = NULL;
    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
    /* Abilita gli interrupt e disabilita il timer */
    setSTATUS((getSTATUS() | STATUS_IEc | STATUS_TE) ^ STATUS_TE);
    WAIT();
    LOG("wait recieved a masked interrupt");
    scheduler_next();
  } else if (procs_count > 0 && !sb_procs) {
    /* DEADLOCK !*/
    LOG("DEADLOCK: panicing");
    PANIC();
  } else {
    LOG("WRONG");
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
  LOG("SCHD");
  memcpy(&result->p_s, statep, sizeof(state_t));
  result->p_pid = pidc++; //(unsigned int)result;

  insert_child(act_proc, result);
  procs_count++;

  enqueue_proc(result, prio);

  return result;
}

inline void kill_proc(pcb_t *p)
{
  --procs_count;
  /* todo: check error on out_blocked */
  if(p->p_parent != NULL && out_child(p) != p)
    PANIC();
  if (p->p_semAdd != NULL && out_blocked(p) == p)
    --sb_procs;
  /*else*/ if (p->p_prio == PROCESS_PRIO_HIGH) {
    out_proc_q(&h_queue, p);
  } else if (p->p_prio == PROCESS_PRIO_LOW) {
    out_proc_q(&l_queue, p);
  }

  free_pcb(p);
}

inline void enqueue_proc(pcb_t *const pcb, const unsigned int priority)
{
  LOGi("ep", pcb->p_pid);
  if (priority == PROCESS_PRIO_HIGH) {
    h_sz++;
    insert_proc_q(&h_queue, pcb);
  } else if (priority == PROCESS_PRIO_LOW) {
    l_sz++;
    insert_proc_q(&l_queue, pcb);
  }
}

inline pcb_t *dequeue_proc(const unsigned int priority)
{
  if (priority == PROCESS_PRIO_HIGH) {
    h_sz--;
    return remove_proc_q(&h_queue);
  } else if (priority == PROCESS_PRIO_LOW) {
    l_sz--;
    return remove_proc_q(&l_queue);
  }
  return NULL;
}

inline void load_proc(pcb_t *pcb)
{
  if (pcb == NULL) {
    LOG("Attempt to load NULL pcb");
    return;
  }
  if (yielded_process != NULL) {
    enqueue_proc(yielded_process, yielded_process->p_prio);
    yielded_process = NULL;
  }
  if (yielded_process != NULL) {
    enqueue_proc(yielded_process, yielded_process->p_prio);
    yielded_process = NULL;
  }

  act_proc = pcb;

  setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
  if (act_proc->p_prio == PROCESS_PRIO_LOW) {
    LOGi("Load lp pro", act_proc->p_pid);
    act_proc->p_s.status ^= STATUS_TE;
  } else if (act_proc->p_prio == PROCESS_PRIO_HIGH) {
    LOGi("Load hp pro", act_proc->p_pid);
  }
  /* Aggiorno l'ultimo update dell'age del processo */
  STCK(act_proc->p_tm_updt);

  /* Lo carico */
  LDST(&act_proc->p_s);
}
