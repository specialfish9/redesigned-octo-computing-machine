#include "exceptions.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "term_utils.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

static unsigned int pid_count = 1;

int create_process(state_t *statep, int prio, support_t *supportp) 
{
  pcb_t *result;

  result = alloc_pcb();
  result->p_supportStruct = supportp;
  result->p_prio = prio;
  result->p_s = *statep;
  result->p_pid = ++pid_count;

  return result->p_pid;
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

