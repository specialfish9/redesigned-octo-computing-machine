#include "asl.h"
#include "pcb.h"

#define DEV_NUM 10 /* TODO */

static size_tt alive_processes;
static size_tt blocked_processes;
static pcb_t *active_process;
static int devices_sem[DEV_NUM];
static struct list_head ready_queue;

void init_data_structures(void) {
  init_pcbs();
  init_asl();

  alive_processes = 0;
  blocked_processes = 0;
  active_process = NULL;
  mk_empty_proc_q(&ready_queue);

  /* TODO pass up vec */
}

void init_devices(void) { /* TODO */}

void init_scheduler(void) { /* TODO */}

int main(int argc, char *argv[])
{
  init_data_structures();

  init_devices();

  init_scheduler();
  
  return 0;
}
