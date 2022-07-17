
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


#define LOG(s) log("SS", s)
#define LOGi(s, i) logi("SS", s, i)

/**
 * @brief Gestore delle systemcall a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
static void support_syscall_handler(support_t* act_proc_sup);
/**
 * @brief Systemcall GET TOD (SYS1). Restituisce il valore di microsecondi dall'avvio del sistema
 * @return valore in microsecondi dall'avvio del sistema
 * */
static unsigned int get_TOD(void);
/**
 * @brief Systemcall TERMINATE (SYS2). Termina il processo chiamante attraverso la NSYS2
 * */
static void terminate(void);
/**
 * @brief Systemcall WRITE TO PRINTER (SYS3). Scrive sulla stampante corrispettiva una stringa passata come parametro
 * @param virtAddr indirizzo virtuale del primo carattere da stampare
 * @param len lunghezza della stringa da stampare
 * @param asid id del processo
 * @return Numero di caratteri stampati se ha successo, altrimenti status con segno negativo.
 * */
static int write_to_printer(unsigned int virtAddr, int len, unsigned int asid);
/**
 * @brief Systemcall WRITE TO TERMINAL (SYS4). Scrive sul terminale corrispettivo una stringa passata come parametro
 * @param virtAddr indirizzo virtuale del primo carattere da stampare
 * @param len lunghezza della stringa da stampare
 * @param asid id del processo
 * @return Numero di caratteri stampati se ha successo, altrimenti status con segno negativo.
 * */
static int write_to_terminal(unsigned int virtAddr, int len, unsigned int asid);
/**
 * @brief Systemcall READ FROM TERMINAL (SYS5). Legge un input dal terminale e lo salva in memoria
 * @param virtAddr indirizzo virtuale del buffer dove salvare i caratteri ricevuti
 * @param asid id del processo
 * @return Numero di caratteri ricevuti se ha successo, altrimenti status con segno negativo.
 * */
static int read_from_terminal(unsigned int virtAddr, unsigned int asid);

inline unsigned int get_TOD(void){
    unsigned int ret;
    STCK(ret);
    return ret;
}

inline void terminate(void){
    SYSCALL(TERMPROCESS, act_proc->p_pid, 0, 0);
}

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
        SYSCALL(DOIO, (int)&dev_reg->command, TRANSMITCHAR, 0);
        if(dev_reg->status != READY)
            return -dev_reg->status;
    }
    return i;
}

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
}

inline int read_from_terminal(unsigned int virtAddr, unsigned int asid){
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

void support_exec_handler(void){
    support_t* act_proc_sup = (support_t*)SYSCALL(GETSUPPORTPTR,0,0,0);
    if(act_proc_sup == NULL){
        LOG("Error on get support");
        return;     //TODO gestire l'errore meglio
    }
    unsigned int cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[GENERALEXCEPT].cause);
    if(cause == EXC_SYS){
        support_syscall_handler(act_proc_sup);
    }else {      
        support_trap_handler(act_proc_sup);
    }


    //se exc è syscall
        //support_syscall_handler(act_proc_sup)
    //altrimenti se exc è trap
        //support_trap_handler()
}



void support_syscall_handler(support_t* act_proc_sup){
    unsigned int arg1, arg2; 

    int number = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[GENERALEXCEPT].cause);      
    arg1 = act_proc_sup->sup_exceptState[1].reg_a1;
    arg2 = act_proc_sup->sup_exceptState[1].reg_a2;


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
          LOGi("Unknow syscall ", number);
          PANIC();
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

    if(tmp->p_pid != act_proc->p_pid)
        insert_blocked(&swp_pl_sem, tmp);

    //se il processo tiene mutua esclusione su un semaforo mutex del livello supporto (es. swap pool sem)
        //rilascia la risorsa (NSYS4 / verhogen?) SYSCALL(VERHOGEN,&swp_pl_sem);
    //termina il processo (SYS2)
    SYSCALL(TERMINATE,0,0,0);
}
