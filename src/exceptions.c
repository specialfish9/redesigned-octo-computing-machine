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

extern pcb_t *act_proc;
extern size_tt sb_procs;

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

int handle_syscall(void)
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

  kprint("NSYS");
  kprint_int(number);
  kprint("|");

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
    return 0;
  }
  case PASSEREN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      // problema
      PANIC();
    }
    passeren(sem_addr);
    /* return 0 if act_proc has been blocked */
    return 1;
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
    return 0;
  }
  case GETSUPPORTPTR: {
    act_proc->p_s.reg_a0 = (memaddr)act_proc->p_supportStruct;
  }
  case YIELD: {
    return 1;
  }
  case DOIO: {
    int line = -1, index = -1;
    int *sem;
    int *dev = (int *)DEVICE_FROM_COMDADDR(arg1);
    for (int i = 0; i < N_EXT_IL; ++i) {
      for (int j = 0; j < N_DEV_PER_IL; ++j) {
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
      sem = sem_disk;
    else if (line == IL_ETHERNET - IL_DISK)
      sem = sem_disk;
    else if (line == IL_PRINTER - IL_DISK)
      sem = sem_printer;
    else if (line == IL_TERMINAL - IL_DISK) {
      if (IS_TERM_WRITING(arg1))
        sem = sem_term_out;
      else
        sem = sem_term_in;
    } else {
      kprint("no sem|");
    }
    kprint("here|");
    passeren(sem + index);
    kprint("there|");
    *((unsigned int *)arg1) = arg2;
    return 0;
  }
  // SYSCALL che restituisce in v0 il tempo di utilizzo del processore da parte
  // del processo attivo
  case GETTIME: {
    // p_time nel pcb del processo attivo è costantemente aggiornato durante
    // l'esecuzione, quindi si inserisce quel valore in v0
    act_proc->p_s.reg_v0 = act_proc->p_time;
    break;
  }
  // SYSCALL che inserisce un PID nel registro v0 del processo attivo in base a
  // cosa è scritto in a1
  case GETPROCESSID: {
    int parent = arg1;
    // Se l'argomento 1 è 0 (quindi se parent è falso), in v0 viene inserito il
    // PID del processo chiamante
    if (!parent)
      act_proc->p_s.reg_v0 = act_proc->p_pid;
    // Altrimenti, se l'argomento è diverso da 0, e il processo chiamante ha
    // effettivamente un processo padre, si inserisce in v0 il PID del padre
    else if (act_proc->p_parent != NULL)
      act_proc->p_s.reg_v0 = act_proc->p_parent->p_pid;
    else
      // Come richiesto nella specifica, se viene richiesto il PID del padre di
      // un processo senza genitore, viene restituito 0
      act_proc->p_s.reg_v0 = 0;
    break;
  }
  default:
    /* TODO Any
attempt to request a non-existent Nucleus service should trigger a Program
Trap exception too*/
    return passup_or_die(GENERALEXCEPT);
    break;
  }
  return 1;
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
  /* TODO: forse va modificato il campo p_s->state del pcb per fare lo switch
   * tra running e blocked*/

  /* Se il valore del semaforo è 1 sblocco il processo, se è 0 lo blocco */
  pcb_t *tmp;
  if (*semaddr == 0) {

    // Controlli per bloccare il processo
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      PANIC();
    }
    sb_procs++;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    // Se ci accorgiamo che la risorsa è disponibile ma altri processi la
    // stavano aspettando
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

  return tmp;
}

static void wait_for_clock(void)
{
  /* blocco il processo attivo sul semaforo */
  insert_blocked(&sem_it, act_proc);
  pcb_t *tmp = act_proc;

  /*blocco il processo sul semaforo ricevuto come parametro*/
  int *dev_sem = get_dev_sem(TIMER_SEM_INDEX);
  insert_blocked(dev_sem, tmp);
  tmp = NULL;

  *dev_sem = 1;
  // TODO scheduler_next()
}

static void kill_parent_and_progeny(pcb_t *p)
{
  pcb_t *c;
  while ((c = remove_child(p)) != NULL)
    kill_parent_and_progeny(c);

  kill_proc(p);
}

// Trova l'indice che identifica il device a partire dall'indirizzo del suo
// command register Se non si usa esternamente posso non metterla nel .h giusto?
int get_ind_from_cmd(unsigned int cmd_addr)
{
  int index = ((cmd_addr - DEV_REG_START) / DEV_REG_SIZE);
  // Se l'offset del registro cmd è 4 possiamo restituire direttamente l'indice
  // calcolato
  if ((cmd_addr - DEV_REG_START) % DEV_REG_SIZE == 0x4)
    return index;
  // Altrimenti se è 0xc stiamo esaminando un sub device di trasmissione di un
  // terminale, e quindi andiamo alla categoria successiva di device (+8)
  else if (index > 32 && (cmd_addr - DEV_REG_START) % DEV_REG_SIZE == 0xc)
    return index + DEVPERINT;
  else
    PANIC();
  return -1;
}

int passup_or_die(size_tt kind)
{
  if (act_proc == NULL)
    return 0;
  if (act_proc->p_supportStruct == NULL) {
    kill_parent_and_progeny(act_proc);
    return 0;
  }

  memcpy(act_proc->p_supportStruct->sup_exceptState + kind,
         (state_t *)BIOSDATAPAGE, sizeof(state_t));
  LDCXT(act_proc->p_supportStruct->sup_exceptContext[kind].pc,
        act_proc->p_supportStruct->sup_exceptContext[kind].status,
        act_proc->p_supportStruct->sup_exceptContext[kind].pc);

  /* never reached */
  return 1;
}
