/**
 *
 * @file syscalls.c
 * @brief Implementazione delle funzioni neccessarie per gestire le systemcall.
 *
 * Oltre alle funzioni esposte dal header @ref syscalls.h, contiene
 * l'implementazione di ciascuna syscall con codice da -1 a -10.
 *
 */
#include "syscalls.h"
#include "asl.h"
#include "interrupts.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

#define LOG(s) log("E", s)
#define LOGi(s, i) logi("E", s, i)

/*TODO*/
int debugS;

/**
 * @var Processo Yielded
 * */
pcb_t *yielded_proc;

/**
 * @var Processo in esecuzione
 * */
extern pcb_t *act_proc;

/**
 * @var Numero di processi sb
 * */
extern size_tt sb_procs;

/**
 * @brief Systemcall DOIO (NSYS5)
 * @param cmdaddr Indirizzo del comando
 * @param cmdval Valore del comando
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
static enum eh_act do_io(int *cmdaddr, int cmdval);

/**
 * @brief Systemcall GET TIME (NSYS6). Restituisce in v0 il tempo di utilizzo
 * della CPU del processo.
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 */
static enum eh_act get_time(void);

/**
 * @brief Systemcall WAIT FOR CLOCK (NSYS7)
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
static enum eh_act wait_for_clock(void);

/**
 * @brief Systemcall GET PROCESS PID (NSYS9).Inserisce in v0 il PID del processo
 * chiamante o del suo processo padre
 * @param arg1 (parent): Se è uguale a 0 si scrive il pid del chiamante in v0,
 * altrimenti del padre del chiamante
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
static enum eh_act get_process_pid(const int arg1);

/**
 * @brief Systemcall YIELD (NSYS10).
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
static enum eh_act yield(void);

/**
 * @brief Uccide il processo padre e i processi figli del processo passato
 * come puntatore.
 * @param p Il processo di riferimento.
 * @return L'azione che l'excepton handler deve fare una volta gestita la
 * syscall.
 * */
static void kill_parent_and_progeny(pcb_t *p);

inline enum eh_act handle_syscall(void)
{
  int number;
  unsigned int arg1, arg2, arg3;

  number = (int)act_proc->p_s.reg_a0;
  arg1 = act_proc->p_s.reg_a1;
  arg2 = act_proc->p_s.reg_a2;
  arg3 = act_proc->p_s.reg_a3;

  LOGi("nsys", number);

  if (!is_alive(act_proc)) {
    LOG("Syscall called by a zombie");
    PANIC();
  }

  if (act_proc->p_semAdd != NULL) {
    LOGi("Syscall called by blocked process ", act_proc->p_pid);
    PANIC();
  }

  switch (number) {
  case CREATEPROCESS: {
    return create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
  }
  case TERMPROCESS: {
    pcb_t *res;
    int pid;

    pid = (int)arg1;
    if (!pid) {
      res = act_proc;
    } else {
      res = search_by_pid(pid);
    }

    kill_parent_and_progeny(res);
    return is_alive(act_proc) ? RENQUEUE : NOTHING;
  }
  case PASSEREN: {
    int *sem_addr;

    sem_addr = (int *)arg1;

    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
      LOG("Sem value out of range");
      PANIC();
    }

    if (!is_alive(act_proc)) {
      LOG("Passeren on zombie");
      PANIC();
    }

    return passeren(sem_addr);
  }
  case VERHOGEN: {
    int *sem_addr;

    sem_addr = (int *)arg1;

    if (*sem_addr != 0 && *sem_addr != 1) {
      /* problema */
      LOG("Sem value out of range");
      PANIC();
    }

    if (!is_alive(act_proc)) {
      LOG("verhogen on zombie");
      PANIC();
    }

    return verhogen(sem_addr) == NULL ? NOTHING : RENQUEUE;
  }
  case DOIO: {
    return do_io((int *)arg1, arg2);
  }
  case GETTIME: {
    return get_time();
  }
  case CLOCKWAIT: {
    return wait_for_clock();
  }
  case GETSUPPORTPTR: {
    return get_support();
  }
  case GETPROCESSID: {
    return get_process_pid(arg1);
  }
  case YIELD: {
    return yield();
  }
  default:
    return passup_or_die(GENERALEXCEPT);
  }
}

inline enum eh_act create_process(state_t *statep, int prio,
                                  support_t *supportp)
{
  const pcb_t *const new_proc = mk_proc(statep, prio, supportp);
  if (new_proc == NULL)
    return -1;

  act_proc->p_s.reg_v0 = new_proc->p_pid;

  return CONTINUE;
}

inline int passeren(int *semaddr)
{
  /* Se il valore del semaforo è 1 sblocco il processo, se è 0 lo blocco */
  pcb_t *tmp;

  if (*semaddr == 0) {

    if (!list_empty(&act_proc->p_list))
      list_del(&act_proc->p_list);
    /* Controlli per bloccare il processo */
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      LOG("insert_blocked failed in passeren\n");
      PANIC();
    }
    sb_procs++;
    return NOTHING;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la
     * stavano aspettando */
    enqueue_proc(tmp, tmp->p_prio);
    --sb_procs;
  } else {
    *semaddr = 0;
  }
  return RENQUEUE;
}

inline pcb_t *verhogen(int *semaddr)
{
  /* Se il valore del semaforo è 0 sblocco il processo, se è 1 lo blocco */
  pcb_t *tmp;

  if (*semaddr == 1) {
    if (act_proc == NULL)
      return NULL;

    /* Controlli per bloccare il processo */
    if (!list_empty(&act_proc->p_list))
      list_del(&act_proc->p_list);
    if (insert_blocked(semaddr, act_proc)) {
      /* Se ritorna true non possiamo assegnare un semaforo */
      /* Non dovrebbe mai capitare, ma in caso */
      LOG("insert_blocked failed in verhogen\n");
      PANIC();
    }

    sb_procs++;
    return NULL;
  } else if ((tmp = remove_blocked(semaddr)) != NULL) {
    /* Se ci accorgiamo che la risorsa è disponibile ma altri processi la
     * stavano aspettando */
    enqueue_proc(tmp, tmp->p_prio);
    sb_procs--;
    return tmp;
  } else {
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
  if (act_proc == NULL) {
    LOG("Dying");
    return NOTHING;
  }
  if (act_proc->p_supportStruct == NULL) {
    kill_parent_and_progeny(act_proc);
    LOG("Dying");
    return NOTHING;
  }

  /* Pass up */
  memcpy(act_proc->p_supportStruct->sup_exceptState + kind,
         (state_t *)BIOSDATAPAGE, sizeof(state_t));
  LOGi("Passup", kind);

  /* Just to be safe */
  if (act_proc->p_supportStruct->sup_exceptContext[kind].stackPtr == 0) {
    PANIC();
  }

  LDCXT(act_proc->p_supportStruct->sup_exceptContext[kind].stackPtr,
        act_proc->p_supportStruct->sup_exceptContext[kind].status,
        act_proc->p_supportStruct->sup_exceptContext[kind].pc);

  /* never reached */
  return NOTHING;
}

inline enum eh_act do_io(int *cmdaddr, int cmdval)
{
  size_tt i, j;
  int line = -1, index = -1;
  int *sem;
  int *dev = (int *)DEVICE_FROM_COMDADDR(cmdaddr);

  if (cmdaddr == 0 || cmdaddr == NULL) {
    LOG("DOIO with null or zero command addr");
    PANIC();
  }

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

inline enum eh_act get_time(void)
{
  /* p_time nel pcb del processo attivo è costantemente aggiornato durante */
  /* l'esecuzione, quindi si inserisce quel valore in v0 */
  act_proc->p_s.reg_v0 = act_proc->p_time;
  return CONTINUE;
}

inline enum eh_act wait_for_clock(void) { return passeren(&sem_it); }

inline enum eh_act get_support(void)
{
  act_proc->p_s.reg_v0 = (memaddr)act_proc->p_supportStruct;
  return CONTINUE;
}

inline enum eh_act get_process_pid(const int arg1)
{
  int parent;

  parent = arg1;
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

inline enum eh_act yield(void)
{
  yielded_proc = act_proc;
  return NOTHING;
}
