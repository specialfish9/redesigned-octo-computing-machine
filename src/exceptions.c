#include "exceptions.h"
#include "listx.h"
#include "pcb.h"
#include "term_utils.h"
#include "pandos_const.h"
#include <umps3/umps/libumps.h>

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
}


void unsigned int SYSCALL(unsigned int number, unsigned int arg1, unsigned int arg2, unsigned int arg3)
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