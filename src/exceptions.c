#include "exceptions.h"
#include "asl.h"
#include "interrupts.h"
#include "klog.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>

extern pcb_t *act_proc;

/**
  Crea un nuovo processo come figlio del chiamante.
  @param statep: stato che deve avere il processo.
  @param prio: priorità da assegnare al processo.
  @param supportp: puntatore alla struttura supporto del processo.
  @return Il PID de processo se la syscall ha successo -1 altrimenti
 */
inline static int create_process(state_t *statep, int prio,
                                 support_t *suppportp);

// static void termniate_process(int pid);

/**
  Esegue un'operazione P sul semaforo binario.
  @param semaddr: puntatore al semaforo.
 */
inline static void passeren(int *semaddr);

/**
  Esegue un'operazione V sul semaforo binario.
  @param semaddr: puntatore al semaforo.
 */
inline static void verhogen(int *semaddr);

// static int do_io(int *cmd_addr, int cmd_val);

// static int get_cpu_time(void);

/**
  Esegue un'operazione P sul semaforo di pseudo-clock.
 */
inline static void wait_for_clock(void);

// static support_t* get_support_data(void);

// static int get_proc_id(int parent);

// static int yield(void);

inline static void kill_parent_and_progeny(pcb_t *p);

void handle_syscall(void)
{

  int number;
  unsigned int arg1, arg2, arg3;

  /* luca: schedluer should be round robin, therefore act_prov should be removed
   * from the ready queue it's in and should be enqueued at the back */
  /* for higher priority processes this is not expected, though a good rule of
  thumb is to assert the act_process is outside any queue when an interrupt is
  being handled and having it re-inserted in the right place (either head or
  tail of the queue) when the scheduler takes over */

  number = (int)act_proc->p_s.reg_a0;
  arg1 = act_proc->p_s.reg_a1;
  arg2 = act_proc->p_s.reg_a2;
  arg3 = act_proc->p_s.reg_a3;

  print1("Handling syscall ");
  print1_int(number);

  switch (number) {
  case CREATEPROCESS: {
    int status;

    status = create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
    act_proc->p_s.reg_v0 = status;
    break;
  }
  case TERMPROCESS: {
    pcb_t *res;
    int pid = (int)arg1;
    if (pid == 0)
      res = act_proc;
    else
      res = search_by_pid(pid);

    kill_parent_and_progeny(res);
    break;
  }
  case PASSEREN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      // problema
      PANIC();
    }
    passeren(sem_addr);
    break;
  }
  case VERHOGEN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      // problema
      PANIC();
    }
    verhogen(sem_addr);
    break;
  }
  case CLOCKWAIT: {
    wait_for_clock();
    break;
  }
  case GETSUPPORTPTR: {
    act_proc->p_s.reg_a0 = (memaddr)act_proc->p_supportStruct;
  }
  case YIELD: {
    if (act_proc->p_prio == PROCESS_PRIO_HIGH) {
      rm_proc(act_proc,
              PROCESS_PRIO_HIGH); // TODO: parent a single high priority process
                                  // from causing starvation
      enqueue_proc(act_proc, PROCESS_PRIO_HIGH);

    } else if (act_proc->p_prio == PROCESS_PRIO_LOW) {
      rm_proc(act_proc, PROCESS_PRIO_LOW);
      enqueue_proc(act_proc, PROCESS_PRIO_LOW);
    }
  }
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
      int parent = arg1;
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

static int create_process(state_t *statep, int prio, support_t *supportp)
{
  const pcb_t *const new_proc = mk_proc(statep, prio, supportp);
  if (new_proc == NULL)
    return -1;

  return new_proc->p_pid;
}

static void passeren(int *semaddr)
{
  /* TODO: forse va modificato il campo p_s->state del pcb per fare lo switch tra running e blocked*/

  /* Se il valore del semaforo è 1 sblocco il processo, se è 0 lo blocco */
  pcb_t *tmp;
  if (*semaddr == 0) {  

    // Controlli per bloccare il processo
    if (insert_blocked(semaddr, get_act_proc())) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      PANIC();
    }
    /* TODO: Chiamata a scheduler */

  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    // Se ci accorgiamo che la risorsa è disponibile ma altri processi la
    // stavano aspettando
    enqueue_proc(tmp, tmp->p_prio);
  } else {
    *semaddr = 0;
  }
}

static void verhogen(int *semaddr)
{
  /* Se il valore del semaforo è 0 sblocco il processo, se è 1 lo blocco */
  pcb_t *tmp;
  if (*semaddr == 1) {
    tmp = get_act_proc();
    // Controlli per bloccare il processo
    if (insert_blocked(semaddr, tmp)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      PANIC();
    }
    /* TODO: Chiamata a scheduler */

  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    // Se ci accorgiamo che la risorsa è disponibile ma altri processi la
    // stavano aspettando
    enqueue_proc(tmp, tmp->p_prio);
  } else {
    *semaddr = 1;
  }
}

static void wait_for_clock(void)
{
  kprint("\n---BLOCKING ACTIVE PROCESS ON ASL");

  /* blocco il processo attivo sul semaforo */
  insert_blocked((int *)dev_sem[ITINT], get_act_proc());   /* TODO: usare sem_it da interrupts */

  dev_sem[ITINT] = 1;     /* TODO: anche qua va usato sem_it (anzi forse sta parte di mettere a 1 va tolta ma non sono sicuro) */
  /* TODO: Chiamata a scheduler */
}

static void kill_parent_and_progeny(pcb_t *p)
{
  pcb_t *c;
  while ((c = remove_child(p)) != NULL)
    kill_parent_and_progeny(c);

  kill_proc(p);
}
