#include "asl.h"
#include "exceptions.h"
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

#define DEV_NUM 10 /* TODO */

static int dev_sem[DEV_NUM];
static passupvector_t *passup_vec;

/* FUNZIONI DI INIZIALIZZAZIONE */
inline static void init_passup_vector(void);
inline static void init_data_structures(void);
inline static void init_devices(void);

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

  print1("Init devices...");
  init_devices();
  print1("done!\n");

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
  passup_vec = (passupvector_t *)PASSUPVECTOR;
  passup_vec->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
  passup_vec->exception_handler = (memaddr)exception_handler;
  passup_vec->tlb_refill_stackPtr = KERNELSTACK;
  passup_vec->exception_stackPtr = KERNELSTACK;
}

void init_data_structures(void)
{
  size_tt i;

  init_pcbs();
  init_asl();

  init_scheduler();

  i = 0;
  while (i < DEV_NUM)
    dev_sem[i++] = 0;
}

void init_devices(void)
{
  size_tt i = 0;

  while (i++ < DEV_NUM) {
    dev_sem[i] = 0;
  }
}


void exception_handler(void)
{
  unsigned int cause, KUp;

  cause = CAUSE_GET_EXCCODE(getCAUSE());
  print1("EXCEPTION HANDLER FIRED\n");
  kprint("exc handl called with code: ");
  kprint_hex(cause);
  kprint("|");
  /*

per TLB trap e PROGRAM trap passa il controllo a support struct del processo o
ammaizzalo
   */
  if (cause == EXC_INT) {
    /* Interrupts */
    /* TODO */
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
    memcpy(&act_proc->p_s, (state_t *)BIOSDATAPAGE, sizeof(state_t));
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)act_proc->p_s.reg_a0) < 0) {
      handle_syscall();
      act_proc->p_s.pc_epc = act_proc->p_s.reg_t9 =
          act_proc->p_s.pc_epc + WORD_SIZE;
      LDST(&act_proc->p_s); /* TODO */
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

