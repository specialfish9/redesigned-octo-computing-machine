
#include "sys_support.h"
#include "syscalls.h"
#include "asl.h"
#include "interrupts.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/types.h>
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/cp0.h>
#include "pandos_const.h"
#include "vm_support.h"


#define PRINTCHR 2
void support_syscall_handler(support_t* act_proc_sup);
/*
unsigned int retValue = SYSCALL (GETTOD, 0, 0, 0);
GETTOD=1
*/
inline unsigned int get_TOD(void){
    unsigned int ret;
    STCK(ret);
    return ret;
}

/*
SYSCALL (TERMINATE, 0, 0, 0);
TERMINATE=2
*/
inline void terminate(void){
    SYSCALL(TERMPROCESS, act_proc->p_pid, 0, 0);
}

/*
int retValue = SYSCALL (WRITEPRINTER, char *virtAddr, int len, 0);
WRITEPRINTER=3
*/
inline int write_to_printer(unsigned int virtAddr, int len, unsigned int asid){
    if(len > 128 || len < 0 || virtAddr < KUSEG){
        return SYSCALL(TERMINATE,0,0,0);
    }
    
    dtpreg_t * dev_reg = (dtpreg_t*)DEV_REG_ADDR(IL_PRINTER,asid-1);         //asid è l'id del processo, associazione 1:1 tra processi e devices
    //ciclo che scorre tutta la stringa, inserisce su dev_reg.data0 il carattere attuale, chiama la syscall e poi riesegue
    int i;
    for(i=0; i<len; i++){
        dev_reg->data0 = virtAddr+i;      //carico il carattere da trasmettere sul campo data0, data1 non viene usato
        if(*((char*)(virtAddr+i)) == '\0'){     //fine stringa
            break;
        }
        SYSCALL(DOIO, (int)&dev_reg->command, PRINTCHR, 0);
        if(dev_reg->status != READY)
            return -dev_reg->status;
    }
    return i;
    //TODO bloccare processo chiamante durante la trasmissione


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
inline int write_to_terminal(unsigned int virtAddr, int len, unsigned int asid){
    if(len > 128 || len < 0 || virtAddr < KUSEG){
        return SYSCALL(TERMINATE,0,0,0);
    }

    termreg_t * dev_reg = (termreg_t*)DEV_REG_ADDR(IL_TERMINAL,asid-1);         //asid è l'id del processo, associazione 1:1 tra processi e devices
    int i;
    for(i=0; i<len; i++){
        dev_reg->transm_command = virtAddr+i;      //carico il carattere da trasmettere sul campo data0, data1 non viene usato
        if(*((char*)(virtAddr+i)) == '\0'){
            break;
        }
        SYSCALL(DOIO, (int)&dev_reg->transm_command, TRANSMITCHAR, 0);
        if(dev_reg->transm_status != OKCHARTRANS)
            return -dev_reg->transm_status;
    }
    return i;
    //TODO bloccare processo chiamante durante la trasmissione

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
inline int read_from_terminal(unsigned int virtAddr, unsigned int asid){      //yonas
    //sospende il processo chiamante fino a che una linea di input (stringa) è stata trasmessa dal terminale associato al processo
    //PARAMETRI: indirizzo virtuale di un buffer stringa dove devono essere inseriti i caratteri ricevuti
    //RETURN: restituisce il numero di caratteri trasmessi (se ha avuto successo), altrimenti (status diverso da 5, chatacter received) return dello status del device con segno cambiato
    //l'handler dovrà prendere il valore restituito da questa funzione e piazzarlo nel registro v0 di U-proc
    //NB: i caratteri ricevuti vanno inseriti nel buffer a partire dall'indirizzo ricevuto come parametro in a1
    //ERRORI: se l'indirizzo è fuori dallo spazio logico degli indirizzi del processo: ammazza il processo (SYS2)
    termreg_t * dev_reg = (termreg_t*) DEV_REG_ADDR(IL_TERMINAL, asid-1);
    unsigned int charnstatus;
    unsigned int status;
    int i=0;
    if(virtAddr<KUSEG)
      return SYSCALL(TERMINATE,0,0,0);
     do{
          //Il return della SYSCALL ha nel primo byte lo status, e nel secondo il carattere ricevuto
          charnstatus= SYSCALL(DOIO, (unsigned int)&dev_reg->recv_command,TRANSMITCHAR,0) ;
          status= charnstatus & (0xFF); //maschero il return value per leggere lo status
          if(status!=OKCHARTRANS)
              return -status;
          *((char*)virtAddr++) = charnstatus>>8; //shifto di 8 bit per trattenere soltanto il carattere letto
          i++;
      }while(charnstatus>>8!='\0'); //Da verificare se vogliamo prendere in input anche \0 oppure solo i caratteri effettivi (per ora per sicurezza lo faccio)
      return i;
}








void support_handler(void){
    support_t* act_proc_sup = (support_t*)SYSCALL(GETSUPPORTPTR,0,0,0);
    unsigned int cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[GENERALEXCEPT].cause);
    if(cause == EXC_SYS){
        support_syscall_handler(act_proc_sup);
    }else{      //TODO verificare che questo sia sempre una trap
        support_trap_handler(act_proc_sup);
    }





    //se exc è syscall > 0
        //support_syscall_handler(act_proc_sup) / se è = 0 ci entra lo stesso probabilmente e va nel caso default
    //altrimenti se exc è trap
        //support_trap_handler()
}



void support_syscall_handler(support_t* act_proc_sup){
    unsigned int arg1, arg2, arg3;

    if(act_proc_sup == NULL){           //TODO forse questo controllo va tolto / va messo nel support_handler() perchè viene già fatto a priori dalla passup or die quindi è ridondante
        //LOG("Error on get support");
        return;     //TODO gestire l'errore meglio
    }

    int number = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[GENERALEXCEPT].cause);      
    arg1 = act_proc_sup->sup_exceptState[1].reg_a1;
    arg2 = act_proc_sup->sup_exceptState[1].reg_a2;
    arg3 = act_proc_sup->sup_exceptState[1].reg_a3;


    int ret=-2147483648;          //uso MININT per evitare conflitti con i valori di ritorno che possono essere numeri negativi, 0 e positivi
    switch(number){
        case GETTOD:{
            ret=get_TOD();
            break;
        }
        case TERMINATE:{
            terminate();
            break;
        }
        case WRITEPRINTER:{
            ret=write_to_printer(arg1, arg2, act_proc_sup->sup_asid);
            break;
        }
        case WRITETERMINAL:{
            ret=write_to_terminal(arg1, arg2, act_proc_sup->sup_asid);
            break;
        }
        case READTERMINAL:{
            ret=read_from_terminal(arg1, act_proc_sup->sup_asid);
            break;
        }
        default:{
            //PANIC o qualcosa del genere
        }
    }

    //dopo il completamento con successo della syscall inserisco il valore di ritorno nel registro v0 del processo chiamante
    if(ret != -2147483648)        //uso MININT per evitare conflitti con i valori di ritorno che possono essere numeri negativi, 0 e positivi
        act_proc_sup->sup_exceptState[1].reg_v0 = ret;

    act_proc_sup->sup_exceptState[1].pc_epc = act_proc_sup->sup_exceptState[1].reg_t9 = act_proc_sup->sup_exceptState[1].pc_epc + WORDLEN;      //incremento PC di 4
    

    //TODO after successful completion of syscall place any return status in v0 (fatto) of U-proc and return control to calling process
    enqueue_proc(act_proc, act_proc->p_prio);
    scheduler_next();
}

void support_trap_handler(support_t* act_proc_sup){
    //TODO STA ROBA E' DA CONTROLLARE MOLTO ATTENTAMENTE
    pcb_t *tmp =remove_blocked(&swp_pl_sem);

    if(tmp.p_pid != act_proc.p_pid)
        insert_blocked(&swp_pl_sem, tmp);

    //se il processo tiene mutua esclusione su un semaforo mutex del livello supporto (es. swap pool sem)
        //rilascia la risorsa (NSYS4 / verhogen?)
    //ammazza il processo (SYS2)
    terminate();
}
