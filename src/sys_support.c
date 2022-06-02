
#include "sys_support.h"
#include "kernel.h"
#include "pcb.h"
#include "syscalls.h"
#include "asl.h"
#include "interrupts.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>
#include "pandos_const.h"

/*
unsigned int retValue = SYSCALL (GETTOD, 0, 0, 0);
GETTOD=1
*/
inline unsigned int get_TOD(void){
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    unsigned int ret;
    STCK(ret);
    return ret;

    /*
    Access to the TOD clock value can be accomplished either of the following
    ways:
        • Direct access to the Bus Register memory location: 0x1000.001C
        • Appendix C contains a listing of the µMPS3 System-wide constants file
        contst.h. Included in this file is a pre-defined macro STCK(T) which takes
        an unsigned integer as its input parameter and populates it with the value of
        the low-order word of the TOD clock divided by the Time Scale. [Section
        4.1.1]
    */
}

/*
SYSCALL (TERMINATE, 0, 0, 0);
TERMINATE=2
*/
inline void terminate(void){
/*
 TODO   usare la funzione di terminate della fase 2 al posto di kill_parent_and_progeny
*/

    //spara al processo utente che l'ha chiamato
    pcb_t *proc = act_proc;  //NON SO SE SIA GIUSTO, DEVO CAPIRE SE U-PROC E IL PROCESSO ATTIVO SIANO DUE COSE EQUIVALENTI

    kill_parent_and_progeny(proc);
    //return is_alive(act_proc) ? RENQUEUE : NOTHING;
}

/*
int retValue = SYSCALL (WRITEPRINTER, char *virtAddr, int len, 0);
WRITEPRINTER=3
*/
inline int write_to_printer(char *virtAddr, int len){       //cresta
    //sospende il processo chiamante fino alla fine della trasmissione al printer associato al processo
    //PARAMETRI: indirizzo virtuale del primo carattere della stringa da trasmettere + lunghezza della stringa
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 1, device ready) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //ERRORI: se è chiamato da un indirizzo fuori dallo spazio logico di indirizzi, o con una lunghezza > 128, o con una lunghezza < 0: ammazza il processo (SYS2)

}

/*
int retValue = SYSCALL (WRITETERMINAL, char *virtAddr, int len, 0);
WRITETERMINAL=4
*/
inline int write_to_terminal(char *virtAddr, int len){      //cresta
    //sospende il processo chiamante fino alla fine della trasmissione al terminale associato al processo
    //PARAMETRI: indirizzo virtuale del primo carattere della stringa da trasmettere + lunghezza della stringa
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 5, character transmitted) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //ERRORI: se è chiamato da un indirizzo fuori dallo spazio logico di indirizzi, o con una lunghezza > 128, o con una lunghezza < 0: ammazza il processo (SYS2)

}

/*
int retValue = SYSCALL (READTERMINAL, char *virtAddr, 0, 0);
READTERMINAL=5
*/
inline int read_from_terminal(char *virtAddr){      //yonas
    //sospende il processo chiamante fino a che una linea di input (stringa) è stata trasmessa dal terminale associato al processo
    //PARAMETRI: indirizzo virtuale di un buffer stringa dove devono essere inseriti i caratteri ricevuti
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 5, chatacter received) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //NB: i caratteri ricevuti vanno inseriti nel buffer a partire dall'indirizzo ricevuto come parametro in a1
    //ERRORI: se l'indirizzo è fuori dallo spazio logico degli indirizzi del processo: ammazza il processo (SYS2)
}








void support_handler(void){
    //se exc è syscall > 0
        //support_syscall_handler() / se è = 0 ci entra lo stesso probabilmente e va nel caso default
    //altrimenti se exc è trap
        //support_trap_handler()
}



//HANDLER CHE PROBABILMENTE NON VA; E' SOLO UNA BASE DA CUI PARTIRE
void support_syscall_handler(void){
    int number;
    unsigned int arg1, arg2, arg3;

    number = (int)act_proc->p_s.reg_a0;
    arg1 = act_proc->p_s.reg_a1;
    arg2 = act_proc->p_s.reg_a2;
    arg3 = act_proc->p_s.reg_a3;


    int ret=0;
    switch(number){
        case GETTOD:{
            ret=get_TOD();
        }
        case TERMINATE:{
            terminate();
            break;
        }
        case WRITEPRINTER:{
            ret=write_to_printer((char*)arg1, arg2);
        }
        case WRITETERMINAL:{
            ret=write_to_terminal((char*)arg1, arg2);
        }
        case READTERMINAL:{
            ret=read_from_terminal((char*)arg1);
        }
        default:{
            //PANIC o qualcosa del genere
        }
    }

    if(ret != 0)
        act_proc->p_s.reg_v0 = ret;
    /*
    TODO:
        -after successful completion of syscall place any return status in v0 of U-proc and return control to calling process
        -increment PC by 4
    */

}

void support_trap_handler(void){
    //se il processo tiene mutua esclusione su un semaforo mutex del livello supporto (es. swap pool sem)
        //rilascia la risorsa (NSYS4 / verhogen?)
    //ammazza il processo (SYS2)
}
