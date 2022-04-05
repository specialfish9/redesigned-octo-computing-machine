#include "exceptions.h"
#include "pandos_types.h"
#include "scheduler.h"

extern pcb_t *act_proc;

int create_process(state_t *statep, int prio, support_t *supportp)
{
  const pcb_t *const new_proc = mk_proc(statep, prio, supportp);
  if (new_proc == NULL)
    return -1;

  act_proc->p_s.reg_v0 = new_proc->p_pid;
  return new_proc->p_pid;
}

void passeren(int *semaddr)
{
  /*https://it.wikipedia.org/wiki/Semaforo_(informatica)#:~:text=Esempi%20di%20uso%20di%20semafori%5Bmodifica%20%7C%20modifica%20wikitesto%5D*/
  (*semaddr)--;
  if ((*semaddr) > 0) {
    // metto un processo in coda dagli attivi
  }
}

void verhogen(int *semaddr)
{
  (*semaddr)++;
  // rimuovo il primo processo e lo rendo attivo
}

int wait_for_clock()
{
  // passeren su semaforo di interval timer + blocca processo invocante fino a
  // prossimo tick del dispositivo
  return 0;
}
