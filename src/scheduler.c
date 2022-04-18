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
/** Coda a bassa priorità */
struct list_head l_queue;
/** Coda ad alta priorità */
struct list_head h_queue;

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
    LOG("!!! Imp all init");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = entry_point;
  proc->p_s.status = ALLOFF | STATUS_TE | STATUS_IM_MASK | STATUS_KUc |  STATUS_IEp;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = pidc++; //(memaddr)proc;

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

static void log_status(void)
{
  // kprint("process_count ");
  // kprint_int(procs_count);
  // kprint("\n");
  // kprint("sb_count ");
  // kprint_int(sb_procs);
  // kprint("\n");
}

inline static void print_queue(const char *prefix, struct list_head *h, int max)
{
  kprint("S>[");
  struct list_head *ptr;
  list_for_each(ptr, h)
  {
    pcb_t *pcb = container_of(ptr, pcb_t, p_list);
    kprint_int((unsigned int)pcb);
    kprint((char *)prefix);
    kprint(",");
    if(max--<0)
      PANIC();
  }
  kprint("]\n");
}

inline void scheduler_next(void)
{
  pcb_t *next_proc;

  log_status();

  print_queue("h", &h_queue, 20);
  print_queue("l", &l_queue, 20);
  fn();
  if (!empty_proc_q(&h_queue)) {
    /* Scegli un processo a priorità alta */
    next_proc = dequeue_proc(PROCESS_PRIO_HIGH);
    if (&(next_proc->p_s) == NULL) {
      kprint("!!! Something wrong with high prior queue. Panicing...\n");
      PANIC();
    }

    load_proc(next_proc);
  } else if (!empty_proc_q(&l_queue) ) {
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
    kprint("WAIT()\n");
    kprint("process_count = ");
    kprint_int(procs_count);
    kprint("\n");
    kprint("sb_procs = ");
    kprint_int(sb_procs);
    kprint("\n");
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
  memcpy(&result->p_s, statep, sizeof(state_t));
  result->p_pid = pidc++; //(unsigned int)result;

  insert_child(act_proc, result);
  procs_count++;

  enqueue_proc(result, prio);

  return result;
}

inline void kill_proc(pcb_t *p)
{
  if(p == NULL){
    kprint("killing a null process\n");
    PANIC();
  }
  kprint("KILL(");
  kprint_int(p->p_pid);
  kprint(")");
  // print_queue("KP", &l_queue);
  --procs_count;
  if (p->p_parent != NULL && out_child(p) != p) {
    PANIC();
  }
  if (p->p_semAdd != NULL){
    --sb_procs;
    if( out_blocked(p) == NULL) {
      kprint("!!!!!! could not remove process from semaphore ");
      kprint_int(p->p_pid);
      kprint("\n");
      PANIC();
    }
  }
  /*else*/ if (p->p_prio == PROCESS_PRIO_HIGH) {
    out_proc_q(&h_queue, p);
  } else if (p->p_prio == PROCESS_PRIO_LOW) {
    LOG("deleting lp");
    out_proc_q(&l_queue, p);
  }

  free_pcb(p);
}

inline void enqueue_proc(pcb_t *const pcb, const unsigned int priority)
{
  kprint("ENQUEUE(");
  kprint_int(pcb->p_pid);
  kprint(")\n");
  if (priority == PROCESS_PRIO_HIGH) {
    insert_proc_q(&h_queue, pcb);
  } else if (priority == PROCESS_PRIO_LOW) {
    insert_proc_q(&l_queue, pcb);
  }
}

inline pcb_t *dequeue_proc(const unsigned int priority)
{
  pcb_t*p=NULL;
  if (priority == PROCESS_PRIO_HIGH) {
    p= remove_proc_q(&h_queue);
  } else if (priority == PROCESS_PRIO_LOW) {
    p= remove_proc_q(&l_queue);
  }
  kprint("DEQUEUE(): ");
  kprint_hex((unsigned int)p);
  kprint("\n");
  return p;
}

inline void load_proc(pcb_t *pcb)
{
  if (pcb == NULL) {
    LOG("Attempt to load NULL pcb");
    return;
  }
  if (yielded_process != NULL) {
    LOG("enqueueing yielded process");
    enqueue_proc(yielded_process, yielded_process->p_prio);
    yielded_process = NULL;
  }

  act_proc = pcb;
  // kprint("act_proc->pSemAdd = ");
  // kprint_hex((unsigned int)act_proc->p_semAdd);
  // kprint("\n");

setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
  act_proc->p_s.status |= STATUS_IEp | STATUS_TE;
  if (act_proc->p_prio == PROCESS_PRIO_LOW) {
    kprint("LOAD(");
    kprint_int(act_proc->p_pid);
    kprint(", low)\n");
  } else if (act_proc->p_prio == PROCESS_PRIO_HIGH) {
    kprint("LOAD(");
    kprint_int(act_proc->p_pid);
    kprint(", high)\n");
    act_proc->p_s.status ^= STATUS_TE;
  }
  /* Aggiorno l'ultimo update dell'age del processo */
  STCK(act_proc->p_tm_updt);

  /* Lo carico */
  LDST(&act_proc->p_s);
}
