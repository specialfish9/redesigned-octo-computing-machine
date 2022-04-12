
#include "exceptions.h"
#include "interrupts.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "asl.h"
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
  //print1("Init passup vector...");
  init_passup_vector();
  //print1("done!\n");
  kprint("Init pv done|");

  //print1("Init data structures...");
  init_data_structures();
  //print1("done!\n");
  kprint("Init data str done|");

  //print1("Loading interval timer...");
  LDIT(100000); /* 100 millisecs */
  //print1("done!\n");
  kprint("IT load done|");

      setSTATUS((getSTATUS() | STATUS_IEc | STATUS_TE | STATUS_IM_MASK) ^ STATUS_TE);


  //print1("Creating init process...");
  create_init_proc((memaddr)test);
  //print1("done!\n");
  kprint("Init proc done|");

  //print1("Starting init process...\n");
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
  cpu_t now;
  state_t *saved_state;
  int reenqueue = 1;

  /* Aggiorno l'età del processo attivo */
  STCK(now);
  act_proc->p_time += (now - act_proc->p_tm_updt);
  act_proc->p_tm_updt = now;

  cause = CAUSE_GET_EXCCODE(getCAUSE());
  //print1("EXCEPTION HANDLER FIRED with code ");
  //print1_int(cause);

  if(cause != 8) {
    kprint("EXH");
    kprint_hex(cause);
    kprint("|");
  }

  saved_state = (state_t *)BIOSDATAPAGE;
  /*
per TLB trap e PROGRAM trap passa il controllo a support struct del processo o
ammaizzalo
   */
   /* vi serve un modo per decidere se un processo deve essere inserito in coda ai processi ready o il controllo deve essere preservato dallo scheduler.
   dovreste aggiungere un'altra variabile un po' come `reenqueue` e passarla allo scheduler che intal caso dovrebbe subito far partire
   l'act_process */
  memcpy(&act_proc->p_s, saved_state, sizeof(state_t));
  if (cause == EXC_INT) {
    /* Interrupts */
    /* Controlla se ci sono interrupt su tutte le linee */
    size_tt i = 0;
    while (i < DEVINTNUM + 1) {
      if (!(getCAUSE() & CAUSE_IP(i)))
        handle_interrupts(i);
    }
  } else if (cause == EXC_MOD || cause == EXC_TLBL || cause == EXC_TLBS) {
      reenqueue = passup_or_die(PGFAULTEXCEPT);

    /* TLB trap */
    /* TODO */
    //} else if (cause >= 4 && cause <= 7) || (cause >= 9 && cause <= 12)) {
  } else if (cause == EXC_ADEL || cause == EXC_ADES || cause == EXC_IBE ||
             cause == EXC_DBE || cause == EXC_BP || cause == EXC_RI ||
             cause == EXC_CPU || cause == EXC_OV) {
              reenqueue = passup_or_die(GENERALEXCEPT);
  } else if (cause == EXC_SYS) {
    /* Syscall */
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)act_proc->p_s.reg_a0) < 0) {
      reenqueue=handle_syscall();

      /* Incrementiamo il PC */
      act_proc->p_s.pc_epc = act_proc->p_s.reg_t9 =
          saved_state->pc_epc + WORD_SIZE;
    } else {
      /*  todo: set trap code */
      reenqueue =  passup_or_die(GENERALEXCEPT);
    }
  }
  if(reenqueue)
    enqueue_proc(act_proc, act_proc->p_prio);
  kprint("rescheduling|");
  scheduler_next();
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
