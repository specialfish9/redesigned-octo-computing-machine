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
    // SYSCALL che si occupa di avviare una richiesta di I/O
    case DOIO: {
      // unsigned int sem=
      // passeren(sem)
      break;
    }
    //SYSCALL che restituisce in v0 il tempo di utilizzo del processore da parte del processo attivo
    case GETTIME: {
      //p_time nel pcb del processo attivo è costantemente aggiornato durante l'esecuzione, quindi si inserisce quel valore in v0
      act_proc->p_s.reg_v0=act_proc->p_time;
      break;
    }
    //SYSCALL che inserisce un PID nel registro v0 del processo attivo in base a cosa è scritto in a1
    case GETPROCESSID: {
      int parent= *arg1;
      //Se l'argomento 1 è 0 (quindi se parent è falso), in v0 viene inserito il PID del processo chiamante
      if(!parent)
        act_proc->p_s.reg_v0=act_proc->p_pid;
      //Altrimenti, se l'argomento è diverso da 0, e il processo chiamante ha effettivamente un processo padre, si inserisce in v0 il PID del padre
      else if(act_proc->p_parent!=NULL)
        act_proc->p_s.reg_v0=act_proc->p_parent->p_pid;
      else
      //Come richiesto nella specifica, se viene richiesto il PID del padre di un processo senza genitore, viene restituito 0
        act_proc->p_s.reg_v0=0;
      break;
    }
    default:
      /* TODO Any
  attempt to request a non-existent Nucleus service should trigger a Program
  Trap exception too*/
      break;
  }
}
