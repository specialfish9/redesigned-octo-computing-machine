#include "kernel.h"
#include "asl.h"
#include "interrupts.h"
#include "pcb.h"
#include "scheduler.h"
#include "syscalls.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

/* Macro per il log */
#define LOG(s) log("K", s)
#define LOGi(s, i) logi("K", s, i);

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
  yielded_proc = NULL;
}

void exception_handler(void)
{
  unsigned int cause, KUp;
  cpu_t now;
  int reenqueue = RENQUEUE;
  size_tt i;

  cause = CAUSE_GET_EXCCODE(getCAUSE());

  if (act_proc != NULL) {
    state_t *saved_state = (state_t *)BIOSDATAPAGE;
    memcpy(&act_proc->p_s, saved_state, sizeof(state_t));
  }

  if (cause == EXC_INT) {
    /* Interrupts */
    /* Controlla se ci sono interrupt su tutte le linee */
    i = 0;
    while (i < N_INTERRUPT_LINES) {
      if (getCAUSE() & CAUSE_IP(i)) {
        reenqueue = handle_interrupts(i);
        break;
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
    if (act_proc == NULL) {
      LOG("Syscall called with no active process. Panicing...");
      /* Syscall chiamata quando nessun processo è in esecuzione. */
      PANIC();
    }
    /* Syscall */
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)act_proc->p_s.reg_a0) < 0) {
      reenqueue = handle_syscall();

      /* Incrementiamo il PC */
      act_proc->p_s.pc_epc = act_proc->p_s.reg_t9 =
          act_proc->p_s.pc_epc + WORD_SIZE;
    } else {
      act_proc->p_s.cause =
          (act_proc->p_s.cause & CLEAREXECCODE) | (PRIVINSTR << CAUSESHIFT);
      reenqueue = passup_or_die(GENERALEXCEPT);
    }
  }
  /* Aggiorno l'età del processo attivo */
  if (act_proc != NULL) {
    STCK(now);
    act_proc->p_time += (now - act_proc->p_tm_updt);
    act_proc->p_tm_updt = now;
  }

  if (act_proc != NULL) {
    if (reenqueue == CONTINUE) {
      load_proc(act_proc);
    } else if (reenqueue == RENQUEUE) {
      enqueue_proc(act_proc, act_proc->p_prio);
    }
  }
  scheduler_next();
}

/* TLB-Refill Handler */
void uTLB_RefillHandler(void)
{
  LOG("TLB refill handler invoked");
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();

  LDST((state_t *)BIOSDATAPAGE);
}
