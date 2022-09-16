#include "sys_support.h"
#include "interrupts.h"
#include "pager.h"
#include "pandos_const.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

/* Macro per il log */
#define LOG(s) log("sS", s)
#define LOGi(s, i) logi("sS", s, i)

#define INTMIN -2147483648

/**
 * @var Semafori per l'accesso in mutua esclusione ai device. Usati dal livello
 * supporto. Vengono inizializzati con @ref init_supp_structure.
 * */
extern int dev_sems[DEVINTNUM + 1][DEVPERINT];

/**
 * @brief Gestore delle systemcall a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
static inline void support_syscall_handler(support_t *act_proc_sup);

/**
 * @brief Gestore per le trap a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
static inline void support_trap_handler(support_t *act_proc_sup);

/**
 * @brief Systemcall GET TOD (SYS1). Restituisce il valore di microsecondi
 * dall'avvio del sistema
 * @return valore in microsecondi dall'avvio del sistema
 * */
static inline unsigned int get_TOD(void);

/**
 * @brief Systemcall TERMINATE (SYS2). Termina il processo chiamante attraverso
 * la NSYS2
 * */
static inline void terminate();

/**
 * @brief Systemcall WRITE TO PRINTER (SYS3). Scrive sulla stampante
 * corrispettiva una stringa passata come parametro
 * @param virtAddr indirizzo virtuale del primo carattere da stampare
 * @param len lunghezza della stringa da stampare
 * @param asid id del processo
 * @return Numero di caratteri stampati se ha successo, altrimenti status con
 * segno negativo.
 * */
static inline int write_to_printer(unsigned int virtAddr, int len,
                                   unsigned int asid);

/**
 * @brief Systemcall WRITE TO TERMINAL (SYS4). Scrive sul terminale
 * corrispettivo una stringa passata come parametro
 * @param virtAddr indirizzo virtuale del primo carattere da stampare
 * @param len lunghezza della stringa da stampare
 * @param asid id del processo
 * @return Numero di caratteri stampati se ha successo, altrimenti status con
 * segno negativo.
 * */
static inline int write_to_terminal(unsigned int virtAddr, int len,
                                    unsigned int asid);

/**
 * @brief Systemcall READ FROM TERMINAL (SYS5). Legge un input dal terminale e
 * lo salva in memoria
 * @param virtAddr indirizzo virtuale del buffer dove salvare i caratteri
 * ricevuti
 * @param asid id del processo
 * @return Numero di caratteri ricevuti se ha successo, altrimenti status con
 * segno negativo.
 * */
static inline int read_from_terminal(unsigned int virtAddr, unsigned int asid);

unsigned int get_TOD(void)
{
  unsigned int ret;
  STCK(ret);
  return ret;
}

void terminate(void)
{
  int pid;

  /* Recupero il pid usando la NSYS9 */
  pid = SYSCALL(GETPROCESSID, 0, 0, 0);

  /* Uccido il processo usando NSYS2 */
  SYSCALL(TERMPROCESS, pid, 0, 0);
}

int write_to_printer(unsigned int virtAddr, int len, unsigned int asid)
{
  dtpreg_t *dev_reg;
  size_tt i;
  unsigned int status;

  if (len > 128 || len < 0 || virtAddr < KUSEG) {
    return SYSCALL(TERMINATE, 0, 0, 0);
  }

  /* Richiedo la mutua esclusione */
  SYSCALL(PASSEREN, (unsigned int)&dev_sems[PRINTER_SEMS][asid - 1], 0, 0);

  /* asid è l'id del processo, associazione 1:1 tra processi e devices */
  dev_reg = (dtpreg_t *)DEV_REG_ADDR(IL_PRINTER, asid - 1);

  /* ciclo che scorre tutta la stringa, inserisce su dev_reg.data0 il carattere
   attuale, chiama la syscall e poi riesegue */
  for (i = 0; i < len; i++) {
    /* carico il carattere da trasmettere sul campo data0, data1 non viene usato
     */
    dev_reg->data0 = *((char *)(virtAddr + i));

    if (*((char *)(virtAddr + i)) == '\0') { // fine stringa
      break;
    }

    status = SYSCALL(DOIO, (int)&dev_reg->command, TRANSMITCHAR, 0);

    if (status != READY) {
      /* Rilascio la mutua esclusione */
      SYSCALL(VERHOGEN, (unsigned int)&dev_sems[PRINTER_SEMS][asid - 1], 0, 0);
      return -dev_reg->status;
    }
  }

  /* Rilascio la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&dev_sems[PRINTER_SEMS][asid - 1], 0, 0);
  return i;
}

int write_to_terminal(unsigned int virtAddr, int len, unsigned int asid)
{
  termreg_t *dev_reg;
  unsigned int status;
  size_tt i;
  unsigned int cmdval;

  if (len > 128 || len < 0 || virtAddr < KUSEG) {
    return SYSCALL(TERMINATE, 0, 0, 0);
  }

  /* Richiedo la mutua esclusione */
  SYSCALL(PASSEREN, (unsigned int)&dev_sems[TERMOUT_SEMS][asid - 1], 0, 0);

  /* asid è l'id del processo, associazione 1:1 tra processi e devices */
  dev_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, asid - 1);

  for (i = 0; i < len; i++) {
    /* carico il carattere da trasmettere sul campo data0, data1 non viene usato
     */
    cmdval = (*(char *)(virtAddr + i) << 8) | TRANSMITCHAR;

    if (*((char *)(virtAddr + i)) == '\0') {
      break;
    }

    status = SYSCALL(DOIO, (int)&dev_reg->transm_command, cmdval, 0);

    if ((status & TERMSTATMASK) != OKCHARTRANS) {
      /* Rilascio la mutua esclusione */
      SYSCALL(VERHOGEN, (unsigned int)&dev_sems[TERMOUT_SEMS][asid - 1], 0, 0);
      return -dev_reg->transm_status;
    }
  }

  /* Rilascio la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&dev_sems[TERMOUT_SEMS][asid - 1], 0, 0);

  return i;
}

int read_from_terminal(unsigned int virtAddr, unsigned int asid)
{
  termreg_t *dev_reg;
  unsigned int status;
  size_tt i = 0;

  if (virtAddr < KUSEG)
    safe_kill();

  /* Richiedo la mutua esclusione */
  SYSCALL(PASSEREN, (unsigned int)&dev_sems[TERMIN_SEMS][asid - 1], 0, 0);

  dev_reg = (termreg_t *)DEV_REG_ADDR(IL_TERMINAL, asid - 1);

  do {
    /* Il return della SYSCALL ha nel primo byte lo status, e nel secondo il
     carattere ricevuto */
    status =
        SYSCALL(DOIO, (unsigned int)&dev_reg->recv_command, TRANSMITCHAR, 0);

    /* maschero il return value per leggere lo status */
    if ((status & TERMSTATMASK) != OKCHARTRANS) {
      /* Rilascio la mutua esclusione */
      SYSCALL(VERHOGEN, (unsigned int)&dev_sems[TERMIN_SEMS][asid - 1], 0, 0);
      return -status;
    }

    /* shifto di 8 bit per trattenere soltanto il carattere letto */
    *((char *)virtAddr++) = status >> 8;
    i++;

  } while (status >> 8 != '\n');

  /* Rilascio la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&dev_sems[TERMIN_SEMS][asid - 1], 0, 0);

  return i;
}

void support_exec_handler(void)
{
  support_t *act_proc_sup;
  unsigned int cause;

  act_proc_sup = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if (act_proc_sup == NULL) {
    LOG("Error on get support");
    return;
  }

  cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[GENERALEXCEPT].cause);

  /* Se e' una syscall */
  if (cause == EXC_SYS) {
    support_syscall_handler(act_proc_sup);
  } else {
    /* Altrimenti la gestiamo come trap */
    support_trap_handler(act_proc_sup);
  }
}

void support_syscall_handler(support_t *act_proc_sup)
{
  unsigned int arg1, arg2;
  int number, ret;

  number = act_proc_sup->sup_exceptState[1].reg_a0;
  arg1 = act_proc_sup->sup_exceptState[1].reg_a1;
  arg2 = act_proc_sup->sup_exceptState[1].reg_a2;

  /* uso INTMIN per evitare conflitti con i valori di ritorno che possono
   * essere numeri negativi, 0 e positivi */
  ret = INTMIN;

  switch (number) {
  case GETTOD: {
    ret = get_TOD();
    break;
  }
  case TERMINATE: {
    terminate();
    break;
  }
  case WRITEPRINTER: {
    ret = write_to_printer(arg1, arg2, act_proc_sup->sup_asid);
    break;
  }
  case WRITETERMINAL: {
    ret = write_to_terminal(arg1, arg2, act_proc_sup->sup_asid);
    break;
  }
  case READTERMINAL: {
    ret = read_from_terminal(arg1, act_proc_sup->sup_asid);
    break;
  }
  default: {
    LOG("Attempt to do a syscall >5");
    safe_kill();
  }
  }

  /* Dopo il completamento con successo della syscall inserisco il valore di
     ritorno nel registro v0 del processo chiamante */
  if (ret != INTMIN) {
    act_proc_sup->sup_exceptState[1].reg_v0 = ret;
  }

  /* Incremento PC di 4 */
  act_proc_sup->sup_exceptState[1].pc_epc =
      act_proc_sup->sup_exceptState[1].reg_t9 =
          act_proc_sup->sup_exceptState[1].pc_epc + WORDLEN;

  /* Lascio il controllo al processo corrente caricando lo stato salvato */
  LDST(&act_proc_sup->sup_exceptState[1]);
}

void support_trap_handler(support_t *act_proc_sup) { safe_kill(); }

inline void safe_kill(void) { SYSCALL(TERMPROCESS, 0, 0, 0); }
