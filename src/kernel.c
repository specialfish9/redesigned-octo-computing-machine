#include "asl.h"
#include "pcb.h"
#include <umps3/umps/types.h>
#include "term_utils.h"

#define DEV_NUM 10 /* TODO */

static size_tt alive_processes;
static size_tt softblocked_processes;
static pcb_t *active_process;
static int devices_sem[DEV_NUM];
static struct list_head ready_queue;

static void create_init_proc(pcb_t *proc);
static void init_data_structures(void);
static void init_interval_timer(void);
static void init_devices(void);
static void init_scheduler(void);

extern void test();

int main(int argc, char *argv[])
{
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
  proc->p_s.
  /* TODO init state */
  /* proc->p_s.s_pc = (memaddr) test;
  proc->p_s.s_t9 = (memaddr) test; */

  /* TODO init p_support struct */

  alive_processes++;
  insert_proc_q(&ready_queue, proc); 
  
}

void init_devices(void) { /* TODO */}

void init_scheduler(void) { /* TODO */}

