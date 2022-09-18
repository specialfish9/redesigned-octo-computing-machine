/**
 * @file support.c
 * @brief Modulo contenente le funzioni utili per la gestione del livello supporto
 * */
#include "support.h"
#include "listx.h"
#include "pager.h"
#include "utils.h"
#include "pandos_const.h"
#include "sys_support.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>

#define LOG(s) log("SUP", s)
#define LOGi(s, i) logi("SUP", s, i)

/**
 * @var Master semaphore
 * */
static int master_sem;

/**
 * @var Semafori per l'accesso in mutua esclusione ai device. Usati dal livello
 * supporto. Vengono inizializzati con @ref init_supp_structure.
 * */
static int dev_sems[DEVINTNUM + 1][DEVPERINT];

/**
 * @brief Gestore per le trap a livello supporto
 * @param act_proc_sup struttura di supporto del processo attivo
 * */
static inline void support_trap_handler(support_t *act_proc_sup);


inline void init_supp_structures(void)
{
  size_tt i, j;

  /* Inizializza il semaforo di mutua esclusione della swap pool a 1 */
  sp_sem = 1;

  /* Inizializzo @ref sp_asids */
  sp_asids = 0;

  /* Master semaphore a 0 */
  master_sem = 0;

  /* Imposta tutti i frame della swap pool come 'liberi' */
  for (i = 0; i < SP_SIZE; i++)
    sp_tbl[i].asid = -1;

  /* Inizializza tutti i semafori dei device a 1 */
  for (i = 0; i < DEVINTNUM + 1; i++)
    for (j = 0; j < DEVPERINT; j++)
      dev_sems[i][j] = 1;
}

inline void init_page_table(pteEntry_t *tbl, const int asid)
{
  size_tt i = 0;

  /* Just in case */
  if (tbl == NULL || asid <= 0 || asid > UPROCMAX) {
    LOG("Invalid page table ref or asid");
    PANIC();
  }

  for (i = 0; i < USERPGTBLSIZE - 1; i++) {
    /* Impostiamo il virtual page number e l'asid */
    tbl[i].pte_entryHI = KUSEG + (i << VPNSHIFT) + (asid << ASIDSHIFT);
    tbl[i].pte_entryLO = ENTRYLO_DIRTY;
  }

  /* Se è l'ultima entry (quella associata alla pagina contenente la stack
   * del uproc) settiamo l'indirizzo 0xBFFFF, ovvero il bottom della stack */
  tbl[i].pte_entryHI = KUSEG + GETPAGENO + (asid << ASIDSHIFT);
  tbl[i].pte_entryLO = ENTRYLO_DIRTY;
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

void support_trap_handler(support_t *act_proc_sup) { safe_kill(); }

inline void p_on_dev(const unsigned int type, const unsigned int no)
{
  if (type > DEVINTNUM + 1 || type < 0) {
    LOGi("Tried a P on invalid dev type: ", type);
    PANIC();
  }
  if (no >= DEVPERINT || type < 0) {
    LOGi("Tried a P on invalid dev no: ", no);
    PANIC();
  }

  SYSCALL(PASSEREN, (int) &dev_sems[type][no], 0, 0);
}

inline void v_on_dev(const unsigned int type, const unsigned int no)
{
  if (type > DEVINTNUM + 1 || type < 0) {
    LOGi("Tried a P on invalid dev type: ", type);
    PANIC();
  }
  if (no >= DEVPERINT || type < 0) {
    LOGi("Tried a P on invalid dev no: ", no);
    PANIC();
  }

  SYSCALL(VERHOGEN, (int) &dev_sems[type][no], 0, 0);
}

inline void p_on_master_sem(void)
{
  SYSCALL(PASSEREN, (int) &master_sem, 0, 0);
}

inline void v_on_master_sem(void)
{
  SYSCALL(VERHOGEN, (int) &master_sem, 0, 0);
}
