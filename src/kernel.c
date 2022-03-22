#include "asl.h"
#include "listx.h"
#include "pandos_types.h"
#include "pandos_const.h"
#include "pcb.h"
#include "term_utils.h"
#include "scheduler.h"

#define DEV_NUM 10 /* TODO */

static size_tt procs_count;
static size_tt sb_procs;
static pcb_t *act_proc;
static int dev_sem[DEV_NUM];
static struct list_head ready_queue;
static passupvector_t *passup_vec;

inline static void create_init_proc(pcb_t *proc);
inline static void init_passup_vector(void);
inline static void init_data_structures(void);
inline static void init_interval_timer(void);
inline static void init_devices(void);
inline static void init_scheduler(void);

static void uTLB_RefillHandler(void);
static void exception_handler(void);

extern void test();

int main(int argc, char *argv[])
{
  print1("Init passup vector...");
  init_passup_vector();
  print1("done!\n");

  print1("Init data structures...");
  init_data_structures();
  print1("done!\n");

  print1("Init interval timer...");
  init_interval_timer();
  print1("done!\n");

  print1("Init devices...");
  init_devices();
  print1("done!\n");

  print1("?");
  print1("Init scheduler...");
  init_scheduler();
  print1("done!\n");
  
  print1("Starting init process...\n");
  scheduler_next(act_proc, procs_count, sb_procs);

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
  mk_empty_proc_q(&ready_queue);

  i = 0;
  while (i < DEV_NUM)
    dev_sem[i++] = 0;
}

void init_interval_timer(void)
{
  /* TODO */

}

void create_init_proc(pcb_t *proc)
{
  proc = alloc_pcb();
  /* TODO: capire come diavolo:
   * 1. si abilitano gli interrupt
   * 2. si mette in kernel mode
   */
  proc->p_s.pc_epc = (memaddr)test; /* TODO assicurarsi che pc_epc = s_pc */
  RAMTOP(proc->p_s.reg_sp);
  RAMTOP(proc->p_s.reg_t9);
  proc->p_prio = PROCESS_PRIO_HIGH;

  procs_count++;
  insert_proc_q(&ready_queue, proc);
}

void init_devices(void)
{
  size_tt i= 0;

  while (i++ < DEV_NUM) {
    dev_sem[i] = 0;
  }
}

void init_scheduler(void)
{
  print1("0");
  scheduler_init(&ready_queue);
}

void uTLB_RefillHandler(void)
{ /* place holder */
}

void exception_handler(void)
{ /* place holder */
}
