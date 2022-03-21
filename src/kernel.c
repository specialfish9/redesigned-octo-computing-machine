#include <umps3/umps/const.h>
#include <umps3/umps/types.h>
#include "asl.h"
#include "pandos_types.h"
#include "pcb.h"
#include "term_utils.h"

#define DEV_NUM 10 /* TODO */

static size_tt alive_processes;
static size_tt softblocked_processes;
static pcb_t *active_process;
static int devices_sem[DEV_NUM];
static struct list_head ready_queue;
static passupvector_t *passup_vec;

static void create_init_proc(pcb_t *proc);
static void init_passup_vector(void);
static void init_data_structures(void);
static void init_interval_timer(void);
static void init_devices(void);
static void init_scheduler(void);
static void uTLB_RefillHandler(void);
static void exception_handler(void);

extern void test();

int main(int argc, char *argv[])
{
  print("Init passup vector...");
  init_passup_vector();
  print("done!\n");

  print("Init data structures...");
  init_data_structures();
  print("done!\n");

  print("Init interval timer...");
  init_interval_timer();
  print("done!\n");

  print("Init devices...");
  init_devices();
  print("done!\n");

  print("Init scheduler...");
  init_scheduler();
  print("done!\n");
  
  return 0;
}

void init_passup_vector(void) {
  passup_vec = (passupvector_t*) PASSUPVECTOR;
  passup_vec->tlb_refill_handler = (memaddr) uTLB_RefillHandler;
  passup_vec->exception_handler = (memaddr) exception_handler;
  passup_vec->tlb_refill_stackPtr = KERNELSTACK;
  passup_vec->exception_stackPtr = KERNELSTACK;
}

void init_data_structures(void) {
  size_tt i;

  init_pcbs();
  init_asl();

  alive_processes = 0;
  softblocked_processes = 0;
  active_process = NULL;
  mk_empty_proc_q(&ready_queue);

  i = 0;
  while (i < DEV_NUM) 
    devices_sem[i++] = 0;

}

void init_interval_timer(void){ /* TODO */}

void create_init_proc(pcb_t *proc) {
  proc = alloc_pcb();
  /* TODO: capire come diavolo:
   * 1. si abilitano gli interrupt
   * 2. si mette in kernel mode
   */
  proc->p_s.pc_epc = (memaddr) test; /* TODO assicurarsi che pc_epc = s_pc */
  proc->p_s.reg_sp = RAMSTART; /*TODO assicurarsi che RAMTOP = RAMSTART */
  proc->p_s.reg_t9 = RAMSTART;

  alive_processes++;
  insert_proc_q(&ready_queue, proc); 
  
}

void init_devices(void) { /* TODO */}

void init_scheduler(void) { /* TODO */}

void uTLB_RefillHandler(void) { /* place holder */ }

void exception_handler(void) { /* place holder */ }
