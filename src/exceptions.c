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

pcb_t *yielded_process;

#define LOG(s) kprint("E>" s "\n")

#define LOGi(s, i)                                                             \
  kprint("E>" s);                                                              \
  kprint_int(i);                                                               \
  kprint("\n")

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
 @brief Uccide il processo padre e i processi figli del processo passato
 come puntatore.
 @param p Il processo di riferimento.
*/
inline static void kill_parent_and_progeny(pcb_t *p);

inline static void print_queue(const char *prefix, struct list_head *h) 
{
  kprint("S>[");
  struct list_head* ptr;
  int i = 0;
  list_for_each(ptr, h){
    pcb_t* pcb = container_of(ptr, pcb_t, p_list);
    kprint_int((unsigned int)pcb);
    kprint((char*)prefix);
    kprint(",");
    i++;
  }
  kprint("]");
}

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
    int new_proc_pid =
        create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
    act_proc->p_s.reg_v0 = new_proc_pid;

    print_queue("aa", act_proc->p_prio ? &h_queue : &l_queue);
    LOGi("CREATE PROC", new_proc_pid);
    return CONTINUE;
  }
  case TERMPROCESS: {
    pcb_t *res;

    int pid = (int)arg1;
    if (pid == 0)
      res = act_proc;
    else
      res = search_by_pid(pid);

    kill_parent_and_progeny(res);
    return NOTHING;
  }
  case PASSEREN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
        kprint("!!! sem value out of range");
      PANIC();
    }
    return passeren(sem_addr);
  }
  case VERHOGEN: {
    int *sem_addr = (int *)arg1;
    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
        kprint("!!! sem value out of range");
      PANIC();
    }
      LOGi("sem_val", *sem_addr);
    
    return verhogen(sem_addr) == NULL ? NOTHING : RENQUEUE;
  }
  case DOIO: {
    return do_io((int *)arg1, arg2);
  }
  /* SYSCALL che restituisce in v0 il tempo di utilizzo del processore da parte
 */
  /* del processo attivo */
  case GETTIME: {
    /* p_time nel pcb del processo attivo è costantemente aggiornato durante */
    /* l'esecuzione, quindi si inserisce quel valore in v0 */
      kprint("!!!!!!\n");
      kprint_int(act_proc->p_time);
      kprint("\n!!!!!!\n");
    act_proc->p_s.reg_v0 = act_proc->p_time;
    return CONTINUE;
  }
  case CLOCKWAIT: {
    return passeren(&sem_it);
  }
  case GETSUPPORTPTR: {
    act_proc->p_s.reg_a0 = (memaddr)act_proc->p_supportStruct;
    return CONTINUE;
  }
  /* SYSCALL che inserisce un PID nel registro v0 del processo attivo in base a
   */
  /* cosa è scritto in a1 */
  case GETPROCESSID: {
    int parent = arg1;
    /* Se l'argomento 1 è 0 (quindi se parent è falso), in v0 viene inserito il
     */
    /* PID del processo chiamante */
    if (!parent)
      act_proc->p_s.reg_v0 = act_proc->p_pid;
    /* Altrimenti, se l'argomento è diverso da 0, e il processo chiamante ha */
    /* effettivamente un processo padre, si inserisce in v0 il PID del padre */
    else if (act_proc->p_parent != NULL)
      act_proc->p_s.reg_v0 = act_proc->p_parent->p_pid;
    else
      /* Come richiesto nella specifica, se viene richiesto il PID del padre di
       */
      /* un processo senza genitore, viene restituito 0 */
      act_proc->p_s.reg_v0 = 0;
    return CONTINUE;
  }
  case YIELD: {
    yielded_process = act_proc;
    return NOTHING;
  }
  default:
    return passup_or_die(GENERALEXCEPT);
  }
}

static int create_process(state_t *statep, int prio, support_t *supportp)
{
  pcb_t *const new_proc = mk_proc(statep, prio, supportp);
  if (new_proc == NULL)
    return -1;

  return new_proc->p_pid;
}

inline int passeren(int *semaddr)
{
  /* Se il valore del semaforo è 1 sblocco il processo, se è 0 lo blocco */
  pcb_t *tmp;

  if (*semaddr == 0) {
    LOG("pass1");

    if(!list_empty(&act_proc->p_list))
      list_del(&act_proc->p_list);
    /* Controlli per bloccare il processo */
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      kprint("!!! insert_blocked failed in passeren\n");
      PANIC();
    }
    sb_procs++;
    return NOTHING;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    LOG("pass2");
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la
     * stavano aspettando */
    enqueue_proc(tmp, tmp->p_prio);
    --sb_procs;
  } else {
    LOG("pass3");
    *semaddr = 0;
  }
    return RENQUEUE;
}

inline pcb_t *verhogen(int *semaddr)
{
  /* Se il valore del semaforo è 0 sblocco il processo, se è 1 lo blocco */
  pcb_t *tmp;

  if (*semaddr == 1) {
    LOG("ver1");
    if(act_proc == NULL)
      return NULL;

    /* Controlli per bloccare il processo */
    if(!list_empty(&act_proc->p_list))
      list_del(&act_proc->p_list);
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      kprint("!!! insert_blocked failed in verhogen\n");
      PANIC();
    }

    sb_procs++;
    return NULL;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    LOG("ver2");
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la
     * stavano aspettando */
    enqueue_proc(tmp, tmp->p_prio);
    sb_procs--;
    return tmp;
  } else {
    LOG("ver3");
    *semaddr = 1;
    return act_proc;
  }
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
    return NOTHING;
  if (act_proc->p_supportStruct == NULL) {
    kill_parent_and_progeny(act_proc);
    return NOTHING;
  }

  LOG("POD");
  /* Pass up */
  memcpy(act_proc->p_supportStruct->sup_exceptState + kind,
         (state_t *)BIOSDATAPAGE, sizeof(state_t));
  LDCXT(act_proc->p_supportStruct->sup_exceptContext[kind].pc,
        act_proc->p_supportStruct->sup_exceptContext[kind].status,
        act_proc->p_supportStruct->sup_exceptContext[kind].pc);

  /* never reached */
  return RENQUEUE;
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
  return NOTHING;
}
