#include "asl.h"
#include "klog.h"
#include "pcb.h"
#include "scheduler.h"
#include "term_utils.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

#define DEV_NUM 10 /* TODO */

static size_tt procs_count;
static size_tt sb_procs;
static pcb_t *act_proc;
static int dev_sem[DEV_NUM];
static struct list_head l_queue;
static struct list_head h_queue;
static passupvector_t *passup_vec;

inline static void create_init_proc(void);
inline static void init_passup_vector(void);
inline static void init_data_structures(void);
inline static void init_devices(void);

static void uTLB_RefillHandler(void);
static void exception_handler(void);

extern void test();

int main(void)
{
  print1("Init passup vector...");
  init_passup_vector();
  print1("done!\n");
  kprint("Init pv done");

  print1("Init data structures...");
  init_data_structures();
  print1("done!\n");
  kprint("Init data str done");

  print1("Loading interval timer...");
  LDIT(100000); /* 100 millisecs */
  print1("done!\n");
  kprint("IT load done");

  print1("Init devices...");
  init_devices();
  print1("done!\n");

  print1("Creating init process...");
  create_init_proc();
  print1("done!\n");
  kprint("Init proc done");

  print1("Starting init process...\n");
  scheduler_next(act_proc, procs_count, sb_procs, &h_queue, &l_queue);

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

  procs_count = 0;
  sb_procs = 0;
  act_proc = NULL;
  mk_empty_proc_q(&l_queue);
  mk_empty_proc_q(&h_queue);

  i = 0;
  while (i < DEV_NUM)
    dev_sem[i++] = 0;
}

void create_init_proc(void)
{
  pcb_t *proc;

  if ((proc = alloc_pcb()) == NULL) {
    kprint("Impossible to allocate init process PCB");
    PANIC();
  }

  proc->p_s.pc_epc = proc->p_s.reg_t9 = (memaddr)test;
  proc->p_s.status |= STATUS_TE | STATUS_IM_MASK | STATUS_KUc | STATUS_IEc;
  RAMTOP(proc->p_s.reg_sp);
  proc->p_prio = PROCESS_PRIO_LOW;
  proc->p_pid = 1; // TODO

  procs_count++;
  insert_proc_q(&l_queue, proc);
}

void init_devices(void)
{
  size_tt i = 0;

  while (i++ < DEV_NUM) {
    dev_sem[i] = 0;
  }
}

void exception_handler(void)
{ /* place holder */
  print1("EXCEPTION HANDLER FIRED");
  kprint("exc handl");
  /*
Per distinguere effettivamente di che eccezione si
tratta bisogna leggere il registro Cause.ExcCode:
- 0 = Interrupt
- 1-3 = TLB Trap
- 4-7,9-12 = Program Trap
- 8 = Syscall

per TLB trap e PROGRAM trap passa il controllo a support struct del processo o
ammaizzalo
   */
}
/* TLB-Refill Handler */
/* One can place debug calls here, but not calls to print */
void uTLB_RefillHandler()
{
  print1("TLB refill called");
  kprint("TLB refill called");
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();

  LDST((state_t *)0x0FFFF000);
}
