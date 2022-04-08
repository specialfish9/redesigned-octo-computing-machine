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

/* FUNZIONI DI INIZIALIZZAZIONE */
inline static void init_passup_vector(void);
inline static void init_data_structures(void);

/* HANDLERS */
inline static void uTLB_RefillHandler(void);
inline static void exception_handler(void);

extern void test();

int main(void)
{
  print1("Init passup vector...");
  init_passup_vector();
  print1("done!\n");
  kprint("Init pv done|");

  print1("Init data structures...");
  init_data_structures();
  print1("done!\n");
  kprint("Init data str done|");

  print1("Loading interval timer...");
  LDIT(100000); /* 100 millisecs */
  print1("done!\n");
  kprint("IT load done|");

  print1("Creating init process...");
  create_init_proc((memaddr)test);
  print1("done!\n");
  kprint("Init proc done|");

  print1("Starting init process...\n");
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


void exception_handler(void)
{
  unsigned int cause, KUp;
  state_t *saved_state;

  cause = CAUSE_GET_EXCCODE(getCAUSE());
  print1("EXCEPTION HANDLER FIRED with code ");
  print1_int(cause);

  kprint("EXH");
  kprint_hex(cause);
  kprint("|");

  saved_state = (state_t*) BIOSDATAPAGE;
  /*
per TLB trap e PROGRAM trap passa il controllo a support struct del processo o
ammaizzalo
   */
  if (cause == EXC_INT) {
    /* Interrupts */
    /* Controlla se ci sono interrupt su tutte le linee */
    size_tt i = 0;

    while (i < DEVINTNUM + 1) {
      if(!(getCAUSE() & CAUSE_IP(i))) {
        handle_interrupts(i);
        /* Incrementiamo il PC */
        saved_state->pc_epc = saved_state->reg_t9 = saved_state->pc_epc + WORD_SIZE;
        memcpy(&act_proc->p_s, saved_state, sizeof(state_t));
        LDST(&act_proc->p_s); 
        return; /* Superfluo, ma la sicurezza non è mai troppa */
      }
    }
  } else if (cause == EXC_MOD || cause == EXC_TLBL || cause == EXC_TLBS) {
    /* TLB trap */
    /* TODO */
    //} else if (cause >= 4 && cause <= 7) || (cause >= 9 && cause <= 12)) {
  } else if (cause == EXC_ADEL || cause == EXC_ADES || cause == EXC_IBE ||
             cause == EXC_DBE || cause == EXC_BP || cause == EXC_RI ||
             cause == EXC_CPU || cause == EXC_OV) {
    /* Program trap */
    /* TODO */
  } else if (cause == EXC_SYS) {
    /* Syscall */
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)saved_state->reg_a0) < 0) {
      handle_syscall();

      /* Incrementiamo il PC */
      saved_state->pc_epc = saved_state->reg_t9 = saved_state->pc_epc + WORD_SIZE;
      memcpy(&act_proc->p_s, saved_state, sizeof(state_t));
      LDST(&act_proc->p_s); 
    }
    /*else progran trap TODO */
  }
}

/* TLB-Refill Handler */
void uTLB_RefillHandler(void)
{
  kprint("TLB refill called");
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();

  LDST((state_t *)BIOSDATAPAGE);
}

