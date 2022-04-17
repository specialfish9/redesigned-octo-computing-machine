#include "asl.h"
#include "exceptions.h"
#include "interrupts.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOG(s) kprint("K>" s "\n")
#define LOGi(s, i)                                                             \
  kprint("K>" s);                                                              \
  kprint_int(i);                                                               \
  kprint("\n")

/** @brief Inizializza il passup vector. */
inline static void init_passup_vector(void);

/** Inizializza le strutture dati necessarie per il kernel */
inline static void init_data_structures(void);

/** @brief Gestore delle eccezioni.*/
inline static void exception_handler(void);

/** @brief Gestore del refil del TLB. */
inline static void uTLB_RefillHandler(void);

int main(void)
{
  init_passup_vector();
  LOG("pv done");

  init_data_structures();
  LOG("ds done");

  /* Carichiamo l'interval timer*/
  LDIT(PSECOND);
  LOG("IT loaded");

  /* Impostiamo il registro status */
  setSTATUS((getSTATUS() | STATUS_IEc | STATUS_TE | STATUS_IM_MASK) ^
            STATUS_TE);

  /* Kernel entry point */
  extern void test();
  create_init_proc((memaddr)test);
  LOG("ip created");

  LOG("loading ip");
  scheduler_next();

  return 0;
}

void init_passup_vector(void)
{
  passupvector_t *const passup_vec = (passupvector_t *)PASSUPVECTOR;
  passup_vec->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
  passup_vec->exception_handler = (memaddr)exception_handler;
  passup_vec->tlb_refill_stackPtr = KERNELSTACK;
  passup_vec->exception_stackPtr = KERNELSTACK;
}

void init_data_structures(void)
{
  init_pcbs();
  init_asl();
  init_scheduler();
  init_dev_sem();
}

inline static void print_queue(const char *prefix, struct list_head *h, int max) 
{
  kprint("S>[");
  struct list_head* ptr;
  list_for_each(ptr, h){
    pcb_t* pcb = container_of(ptr, pcb_t, p_list);
    kprint_int((unsigned int)pcb);
    kprint((char*)prefix);
    kprint(",");
    if(max == 0){
      kprint("!!!!!!!!!!!!!!!!!!!!!\n");
      PANIC();
    }
    --max;
  }
  kprint("]");
}

void exception_handler(void)
{
  unsigned int cause, KUp;
  cpu_t now;
  state_t *saved_state;
  int reenqueue = RENQUEUE;
  size_tt i;

  /* Aggiorno l'età del processo attivo */
  if (act_proc != NULL) {
    STCK(now);
    act_proc->p_time += (now - act_proc->p_tm_updt);
    act_proc->p_tm_updt = now;
  }

  cause = CAUSE_GET_EXCCODE(getCAUSE());

  LOGi("EXC", cause);

  if(act_proc!=NULL)
    print_queue("exp", act_proc->p_prio ? &h_queue : &l_queue, 10);

  if (act_proc != NULL) {
    saved_state = (state_t *)BIOSDATAPAGE;
    /* TODO serve un modo per decidere se un processo deve essere inserito in
    coda ai processi ready o il controllo deve essere preservato dallo
    scheduler. bisogna aggiungere un'altra variabile un po' come `reenqueue` e
    passarla allo scheduler che intal caso dovrebbe subito far partire
    l'act_process */
    memcpy(&act_proc->p_s, saved_state, sizeof(state_t));
  }
  if (cause == EXC_INT) {
    /* Interrupts */
    /* Controlla se ci sono interrupt su tutte le linee */
    i = 0;
    while (i < N_INTERRUPT_LINES) {
      if (getCAUSE() & CAUSE_IP(i)) {
        handle_interrupts(i);
      }
      ++i;
    }
  } else if (cause == EXC_MOD || cause == EXC_TLBL || cause == EXC_TLBS) {
    /* TLB trap */
    reenqueue = passup_or_die(PGFAULTEXCEPT);
  } else if (cause == EXC_ADEL || cause == EXC_ADES || cause == EXC_IBE ||
             cause == EXC_DBE || cause == EXC_BP || cause == EXC_RI ||
             cause == EXC_CPU || cause == EXC_OV) {
    reenqueue = passup_or_die(GENERALEXCEPT);
  } else if (cause == EXC_SYS) {
    if (act_proc == NULL){
      /* syscall called in a while state when no process was executing */
      kprint("!!! recieved syscall while act_proc == NULL\n");
      PANIC();
    }
    /* Syscall */
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)act_proc->p_s.reg_a0) < 0) {
      reenqueue = handle_syscall();

      /* Incrementiamo il PC */
      act_proc->p_s.pc_epc = act_proc->p_s.reg_t9 =
          saved_state->pc_epc + WORD_SIZE;
    } else {
      saved_state->cause =
          (saved_state->cause & CLEAREXECCODE) | (PRIVINSTR << CAUSESHIFT);
      reenqueue = passup_or_die(GENERALEXCEPT);
    }
  }

  if(act_proc != NULL) {
    if (reenqueue == CONTINUE) {
      // SUPER MEGA HACK
      load_proc(act_proc);
    } else if (reenqueue == RENQUEUE) {
      enqueue_proc(act_proc, act_proc->p_prio);
    }
  }
  LOG("rschdl");
  scheduler_next();
}

/* TLB-Refill Handler */
void uTLB_RefillHandler(void)
{
  LOG("TLB refill called");
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();

  LDST((state_t *)BIOSDATAPAGE);
}
