#include "exceptions.h"
#include "listx.h"
#include "pandos_const.h"
#include "pcb.h"
#include "term_utils.h"
#include <umps3/umps/libumps.h>

// static int create_process(state_t *statep, int prio, support_t * suppportp);

// static void termniate_process(int pid);

static void passeren(int *semaddr);

static void verhogen(int *semaddr);

// static int do_io(int *cmd_addr, int cmd_val);

// static int get_cpu_time(void);

static int wait_for_clock(void);

// static support_t* get_support_data(void);

// static int get_proc_id(int parent);

// static int yield(void);

static void passeren(int *semaddr)
{
  (*semaddr)--;
  if ((*semaddr) > 0) {
    pcb_t *tmp; //!!! NON SO CHE PROCESSO INSERIRE QUINDI PER ADESSO LASCIO QUESTO !!!
    tmp->p_semAdd = semaddr;
    insert_blocked(semaddr, tmp);   //inserisco il processo sul semaforo indicato come parametro
    tmp = NULL;
  }
}

static void verhogen(int *semaddr)
{
  (*semaddr)++;
  pcb_t *tmp = remove_blocked(semaddr);   //rimuovo un processo bloccato dal semaforo
  if(tmp != NULL){
    //il processo rimosso viene aggiunto alla coda dei processi attivi
    tmp->p_semAdd = NULL;
    //insert_proc_q(/*LISTA DEGLI ATTIVI*/, p);  !!!!!!!!!!!!!!!!!!!!     USARE FUNZIONE FORNITA DA KERNEL PER ASTRARRE L'INTERAZIONE CON LA LISTA
  } 
}

static int wait_for_clock()
{
  // passeren su semaforo di interval timer + blocca processo invocante fino a
  // prossimo tick del dispositivo
  return 0;
}

void handle_syscall(unsigned int number, unsigned int arg1, unsigned int arg2,
                    unsigned int arg3)
{
  switch (number) {
  case PASSEREN:
    passeren(arg1);
    break;
  case VERHOGEN:
    verhogen(arg1);
    break;
  case CLOCKWAIT:
    // int tmp = wait_for_clock();
    break;
  default:
    break;
  }
}
