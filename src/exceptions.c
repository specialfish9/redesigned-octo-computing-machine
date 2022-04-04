#include "exceptions.h"
#include "listx.h"
#include "pcb.h"
#include "term_utils.h"
#include "pandos_const.h"
#include <umps3/umps/libumps.h>

//static int create_process(state_t *statep, int prio, support_t * suppportp);

//static void termniate_process(int pid);

static void passeren(int *semaddr);

static void passeren(int *semaddr);

//static void verhogen(int *semaddr);

//static int do_io(int *cmd_addr, int cmd_val);

//static int get_cpu_time(void);

//static int wait_for_clock(void);

//static support_t* get_support_data(void);

//static int get_proc_id(int parent);

//static int yield(void);

static void passeren(int *semaddr){
    /*https://it.wikipedia.org/wiki/Semaforo_(informatica)#:~:text=Esempi%20di%20uso%20di%20semafori%5Bmodifica%20%7C%20modifica%20wikitesto%5D*/
    (*semaddr)--;
    if((*semaddr)>0){
        //metto un processo in coda dagli attivi
    }
}

static void verhogen(int *semaddr){
    (*semaddr)++;
    //rimuovo il primo processo e lo rendo attivo
}

static int wait_for_clock(){
    //passeren su semaforo di interval timer + blocca processo invocante fino a prossimo tick del dispositivo
  return 0;
}


void handle_syscall(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    switch (number){
        case PASSEREN:
            passeren(arg1); //forse va un puntatore
            break;
        case VERHOGEN:
            verhogen(arg1); //forse va un puntatore
            break;
        case CLOCKWAIT:
            int tmp = wait_for_clock();
            break;
        default:
            break;
    }
}
