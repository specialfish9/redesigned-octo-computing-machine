#include "exceptions.h"
#include "asl.h"
#include "interrupts.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOG(s) kprint("E>" s "|")

/* Processo in esecuzione */
extern pcb_t *act_proc;
/* Numero di processi sb */
extern size_tt sb_procs;

/**
  @brief Crea un nuovo processo come figlio del chiamante.
  @param statep: stato che deve avere il processo.
  @param prio: priorità da assegnare al processo.
  @param supportp: puntatore alla struttura supporto del processo.
  @return Il PID de processo se la syscall ha successo -1 altrimenti
 */
inline static int create_process(state_t *statep, int prio,
                                 support_t *suppportp);

/**
 * @brief do io
 * TODO
 * */
static int do_io(int *cmdaddr, int cmdval);

/**
  @brief Esegue un'operazione P sul semaforo di pseudo-clock.
 */
inline static void wait_for_clock(void);

/**
 @brief Uccide il processo padre e i processi figli del processo passato
 come puntatore.
 @param p Il processo di riferimento.
*/
inline static void kill_parent_and_progeny(pcb_t *p);

inline int handle_syscall(void)
{
  int number;
  unsigned int arg1, arg2, arg3;

  /* for higher priority processes this is not expected, though a good rule of
  thumb is to assert the act_process is outside any queue when an interrupt is
  being handled and having it re-inserted in the right place (either head or
  tail of the queue) when the scheduler takes over */

  number = (int)act_proc->p_s.reg_a0;
  arg1 = act_proc->p_s.reg_a1;
  arg2 = act_proc->p_s.reg_a2;
  arg3 = act_proc->p_s.reg_a3;

  kprint("E>NSYS");
  kprint_int(number);
  kprint("|");

  switch (number) {
  case CREATEPROCESS: {
    act_proc->p_s.reg_v0 =
        create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
    return TRUE;
  }
  case TERMPROCESS: {
    pcb_t *res;

    int pid = (int)arg1;
    if (pid == 0)
      res = act_proc;
    else
      res = search_by_pid(pid);

    kill_parent_and_progeny(res);
    return FALSE;
  }
  case PASSEREN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
      PANIC();
    }
    passeren(sem_addr);
    return TRUE;
  }
  case VERHOGEN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
      PANIC();
    }
    verhogen(sem_addr);
    return TRUE;
  }
  case DOIO: {
    return do_io((int *)arg1, arg2);
  }
  /* SYSCALL che restituisce in v0 il tempo di utilizzo del processore da parte */
  /* del processo attivo */
  case GETTIME: {
    /* p_time nel pcb del processo attivo è costantemente aggiornato durante */
    /* l'esecuzione, quindi si inserisce quel valore in v0 */
    act_proc->p_s.reg_v0 = act_proc->p_time;
    return TRUE;
  }
  case CLOCKWAIT: {
    wait_for_clock();
    return FALSE;
  }
  case GETSUPPORTPTR: {
    act_proc->p_s.reg_a0 = (memaddr)act_proc->p_supportStruct;
  }
  /* SYSCALL che inserisce un PID nel registro v0 del processo attivo in base a */
  /* cosa è scritto in a1 */
  case GETPROCESSID: {
    int parent = arg1;
    /* Se l'argomento 1 è 0 (quindi se parent è falso), in v0 viene inserito il */
    /* PID del processo chiamante */
    if (!parent)
      act_proc->p_s.reg_v0 = act_proc->p_pid;
    /* Altrimenti, se l'argomento è diverso da 0, e il processo chiamante ha */
    /* effettivamente un processo padre, si inserisce in v0 il PID del padre */
    else if (act_proc->p_parent != NULL)
      act_proc->p_s.reg_v0 = act_proc->p_parent->p_pid;
    else
      /* Come richiesto nella specifica, se viene richiesto il PID del padre di */
      /* un processo senza genitore, viene restituito 0 */
      act_proc->p_s.reg_v0 = 0;
    return TRUE;
  }
  case YIELD: {
    return TRUE;
  }
  default:
    return passup_or_die(GENERALEXCEPT);
  }
}

static int create_process(state_t *statep, int prio, support_t *supportp)
{
  const pcb_t *const new_proc = mk_proc(statep, prio, supportp);
  if (new_proc == NULL)
    return -1;

  return new_proc->p_pid;
}

inline void passeren(int *semaddr)
{
  /* Se il valore del semaforo è 1 sblocco il processo, se è 0 lo blocco */
  pcb_t *tmp;
  if (*semaddr == 0) {

    /* Controlli per bloccare il processo */
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      PANIC();
    }
    sb_procs++;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la stavano aspettando */
    --sb_procs;
    enqueue_proc(tmp, tmp->p_prio);
  } else {
    *semaddr = 0;
  }
}

inline pcb_t *verhogen(int *semaddr)
{
  /* Se il valore del semaforo è 0 sblocco il processo, se è 1 lo blocco */
  pcb_t *tmp;

  if (*semaddr == 1) {
    tmp = act_proc;
    /* Controlli per bloccare il processo */
    if (insert_blocked(semaddr, tmp)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      PANIC();
    }

  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la stavano aspettando */
    enqueue_proc(tmp, tmp->p_prio);
  } else {
    *semaddr = 1;
  }

  return tmp;
}

static void wait_for_clock(void)
{
  /* blocco il processo attivo sul semaforo */
  insert_blocked(&sem_it, act_proc);
  pcb_t *tmp = act_proc;

  /*blocco il processo sul semaforo ricevuto come parametro*/
  int *dev_sem = &sem_it;
  insert_blocked(dev_sem, tmp);
  tmp = NULL;

  *dev_sem = 1;
}

static void kill_parent_and_progeny(pcb_t *p)
{
  pcb_t *c;
  while ((c = remove_child(p)) != NULL)
    kill_parent_and_progeny(c);

  kill_proc(p);
}

inline int passup_or_die(size_tt kind)
{
  /* Die */
  if (act_proc == NULL)
    return FALSE;
  if (act_proc->p_supportStruct == NULL) {
    kill_parent_and_progeny(act_proc);
    return FALSE;
  }

  /* Pass up */
  memcpy(act_proc->p_supportStruct->sup_exceptState + kind,
         (state_t *)BIOSDATAPAGE, sizeof(state_t));
  LDCXT(act_proc->p_supportStruct->sup_exceptContext[kind].pc,
        act_proc->p_supportStruct->sup_exceptContext[kind].status,
        act_proc->p_supportStruct->sup_exceptContext[kind].pc);

  /* never reached */
  return TRUE;
}

inline static int do_io(int *cmdaddr, int cmdval)
{
  size_tt i, j;
  int line = -1, index = -1;
  int *sem;
  int *dev = (int *)DEVICE_FROM_COMDADDR(cmdaddr);

  /* Cerco la linea di interrupt e l'indice del device */
  for (i = 0; i < N_EXT_IL; ++i) {
    for (j = 0; j < N_DEV_PER_IL; ++j) {
      if (((int *)DEV_REG_ADDR(IL_DISK + i, j)) != dev)
        continue;

      line = i;
      index = j;
      i = 3 + N_EXT_IL;
      break;
    }
  }

  if (line == IL_DISK - IL_DISK)
    sem = sem_disk;
  else if (line == IL_FLASH - IL_DISK)
    sem = sem_flash;
  else if (line == IL_ETHERNET - IL_DISK)
    sem = sem_net;
  else if (line == IL_PRINTER - IL_DISK)
    sem = sem_printer;
  else if (line == IL_TERMINAL - IL_DISK) {
    if (IS_TERM_WRITING(cmdaddr))
      sem = sem_term_out;
    else
      sem = sem_term_in;
  } else {
    LOG("no sem");
  }
  passeren(sem + index);
  *((unsigned int *)cmdaddr) = cmdval;
  return FALSE;
}
