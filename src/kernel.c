#include "asl.h"
#include "exceptions.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "scheduler.h"
#include "term_utils.h"
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
inline static void handle_syscall(state_t *const saved_state);
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
pcb_t *search_by_pid(int pid){
  pcb_t* tbl=get_free_table();
  for (int i=0; i<MAXPROC; ++i) {
  if(tbl[i].p_pid==pid)
    return &tbl[i];
  }
  return NULL;
}

void kill_parent_and_progeny(pcb_t* p){
  pcb_t* c;
  while((c=remove_child(p))!= NULL)
    kill_progeny(c);

  kill_proc(p);
}

void exception_handler(void)
{
  unsigned int cause, KUp;
  state_t *state;

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
    KUp = ((getSTATUS() & STATUS_KUp) >> STATUS_KUp_BIT);
    state = (state_t *)BIOSDATAPAGE;
    /* Se il processo e' in kernel-mode */
    if (KUp == 0 && ((int)state->reg_a0) < 0) {
      handle_syscall(state);
      LDST(state); /* TODO */
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

void handle_syscall(state_t *const saved_state)
{

  int number;
  unsigned int arg1, arg2, arg3;
  memcpy(&act_proc->p_s, saved_state, sizeof(state_t));

  /* luca: schedluer should be round robin, therefore act_prov should be removed from the ready queue it's in and should be enqueued at the back */
  /* for higher priority processes this is not expected, though a good rule of thumb is to assert the act_process is outside any queue when an interrupt is
  being handled and having it re-inserted in the right place (either head or tail of the queue) when the scheduler takes over */

  number = (int)saved_state->reg_a0;
  arg1 = saved_state->reg_a1;
  arg2 = saved_state->reg_a2;
  arg3 = saved_state->reg_a3;

  switch (number) {
    case CREATEPROCESS: {
      int status;

      status = create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
      saved_state->reg_v0 = status;
      break;
    }
    case TERMPROCESS:{
      pcb_t* res;
      int pid = (int) arg1;
      if(pid==0)
        res=act_proc;
      else
       res=search_by_pid(pid);

      kill_parent_and_progeny(res);
      break;
    }
    case PASSEREN: {
      // passeren(arg1); //forse va un puntatore
      break;
    }
    case VERHOGEN: {
      kprint("veroghen");
      // verhogen(arg1); //forse va un puntatore
      break;
    }
    case CLOCKWAIT: {
      // int tmp = wait_for_clock();
      break;
    }
    case GETSUPPORTPTR:{
      act_proc->p_s.reg_a0=(memaddr)act_proc->p_supportStruct;
    }
    case YIELD:{
      if (act_proc->p_prio == PROCESS_PRIO_HIGH) {
        out_proc_q(&h_queue, act_proc);//TODO: parent a single high priority process from causing starvation
            insert_proc_q(&h_queue, result);

      } else if (act_proc->p_prio == PROCESS_PRIO_LOW) {
        out_proc_q(&l_queue, act_proc);
            insert_proc_q(&l_queue, result);

      }
  }
    default:
      /* TODO Any
  attempt to request a non-existent Nucleus service should trigger a Program
  Trap exception too*/
      break;
  }
}
