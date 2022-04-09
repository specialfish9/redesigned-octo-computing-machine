#include "exceptions.h"
#include "interrupts.h"
#include "pandos_const.h"
#include "scheduler.h"
#include "pcb.h"
#include "utils.h"

extern pcb_t *act_proc;

/**
  Crea un nuovo processo come figlio del chiamante.
  @param statep: stato che deve avere il processo.
  @param prio: priorità da assegnare al processo.
  @param supportp: puntatore alla struttura supporto del processo.
  @return Il PID de processo se la syscall ha successo -1 altrimenti
 */
inline static int create_process(state_t *statep, int prio, support_t *suppportp);

// static void termniate_process(int pid);

inline static void passeren(int *semaddr);

inline static void verhogen(int *semaddr);

// static int do_io(int *cmd_addr, int cmd_val);

// static int get_cpu_time(void);

inline static int wait_for_clock(void);

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
    passeren((int*)arg1); 
    break;
  }
  case VERHOGEN: {
    verhogen((int*)arg1); 
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
      rm_proc(act_proc, PROCESS_PRIO_HIGH); // TODO: parent a single high priority process from causing starvation
      enqueue_proc(act_proc, PROCESS_PRIO_HIGH);

    } else if (act_proc->p_prio == PROCESS_PRIO_LOW) {
      rm_proc(act_proc, PROCESS_PRIO_LOW);
      enqueue_proc(act_proc, PROCESS_PRIO_LOW);
    }
  }
  default:
    /* TODOAny
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

static void passeren(int *semaddr)      //TODO: valutare se questa è la soluzione giusta oppure se serve implementare slide 110 di "concorrenza2122"
{
  /*idea da https://www.geeksforgeeks.org/semaphores-in-process-synchronization/*/
  /*
  if((*semaddr)==1){
    (*semaddr)=0;
  }else{
    pcb_t *tmp = get_act_proc();
    tmp->p_semAdd = semaddr;
    insert_blocked(semaddr, tmp);
    tmp = NULL;
    scheduler_next();
  }
  */


  //depending on the value of the semaphore
  if((*semaddr)==0){    //il processo viene bloccato sulla ASL e viene chiamato lo scheduler
    kprint("\n---SEMAFORO A 0 IN PASSEREN");
    /*estraggo un processo dalla coda degli attivi*/
    pcb_t *tmp = get_act_proc();

    /*blocco il processo sul semaforo ricevuto come parametro*/
    tmp->p_semAdd = semaddr;
    insert_blocked(semaddr, tmp);
    tmp = NULL;

    (*semaddr)=1;
    scheduler_next();
  }else{   //semaforo = 1 quindi restituisco controllo al processo chiamante
    kprint("\n---SEMAFORO A 1 IN PASSEREN");
    return;
  }
}

static void verhogen(int *semaddr)      //TODO: valutare se questa è la soluzione giusta oppure se serve implementare slide 110 di "concorrenza2122"
{
  /*idea da https://www.geeksforgeeks.org/semaphores-in-process-synchronization/*/
  /*
  pcb_t *tmp = remove_blocked(semaddr);  
  if(tmp == NULL){
    (*semaddr) = 1;
  }else{
    tmp->p_semAdd = NULL;
    enqueue_proc(tmp);
  }
  */

  /*rimuovo un processo bloccato dal semaforo*/
  pcb_t *tmp = remove_blocked((int*)semaddr);   
  if(tmp != NULL){    //se posso rimuovere qualcosa dai bloccati lo faccio
    /*il processo rimosso viene aggiunto alla coda dei processi attivi*/
    tmp->p_semAdd = NULL;
    enqueue_proc(tmp);
    (*semaddr)=0;   // !!! NON SO SE QUESTO SIA CONCETTUALMENTE SBAGLIATO !!!
  }else{      //se non posso rimuovere nulla metto il semaforo a 1
    (*semaddr)=1;
  }
}

static int wait_for_clock(void)   //non so cosa deve returnare, bellaraga
{
  passeren((int*)dev_sem[ITINT]);
  //passeren su semaforo di interval timer
  return dev_sem[ITINT];
}

static void kill_parent_and_progeny(pcb_t *p)
{
  pcb_t *c;
  while ((c = remove_child(p)) != NULL)
    kill_parent_and_progeny(c);

  kill_proc(p);
}
