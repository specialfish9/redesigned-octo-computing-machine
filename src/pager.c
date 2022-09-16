/**
 * @file pager.c
 * @brief Implementazione delle funzioni necessarie per la gestione della
 * memoria virtuale.
 * */
#include "pager.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "scheduler.h"
#include "sys_support.h"
#include "syscalls.h"
#include "umps3/umps/cp0.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

/**
 * @brief Dimensione della Swap Pool
 * */
#define SWAP_POOL_SIZE (2 * UPROCMAX)

/**
 * @brief Indirizzo di inizio della Swap Pool
 * */
#define SWAP_POOL_BEGIN 0x20020000

/* Macro per il log */
#define LOG(s) log("VM", s)
#define LOGi(s, i) logi("VM", s, i)

/**
 * @struct swppl_entry_t
 * @brief Rappresenta una entry della swap table.
 * @var asid Indica l'ASID del processo propretario della pagina salvata
 * nella entry
 * @var vpn Indica il numero della pagina salvata nella entry
 * @ pg_tbl_entry Puntatore alla entry dentro la tabella delle pagine privata
 * del processo
 * */
typedef struct {
  int asid;
  int vpn;
  pteEntry_t *pg_tbl_entry;
} swppl_entry_t;

/**
 * @var Array con le entry della swap pool
 * */
static swppl_entry_t swppl_tbl[SWAP_POOL_SIZE];

/**
 * @var Semaforo mutex per la swap pool
 * */
int swp_pl_sem;

/**
 * @var Semafori per l'accesso in mutua esclusione ai device. Usati dal livello
 * supporto. Vengono inizializzati con @ref init_supp_structure.
 * */
int dev_sems[DEVINTNUM + 1][DEVPERINT];

/**
 * @brief Libera un frame salvandone il contenuto nel dispositivo flash
 * @param frame puntatore al frame da liberare
 * @param asid l'asid del processo proprietario del frame
 * */
static inline void free_frame(swppl_entry_t *frame, const int asid);

/**
 * @brief Implementa l'algoritmo di rimpiazzamento per i frame nella swap pool
 * @return Il numero del frame da rimpiazzare
 * */
static inline int chose_frame(void);

/**
 * @brief Utility per abilitare/disabilitare gli interrupt. Usata per poter
 * eseguire istruzioni in modo 'atomico'.
 * @param on Un intero booleano - 1 se si vogliono abilitare gli interrupt, 0
 * se si vogliono disabilitare.
 * */
static inline void toggle_int(int on);

/**
 * @brief Funzione per leggere da un dispositivo flash
 * @param asid L'asid del processo
 * @param pg_no Il numero della pagina da leggere
 * @param dest L'indirizzo su cui salvare i byte letti
 * @return Lo status della operazione
 * */
static inline unsigned int read_from_flash(const int asid,
                                           const unsigned int pg_no,
                                           const unsigned int dest);

/**
 * @brief Funzione per scrivere su un dispositivo flash
 * @param asid L'asid del processo
 * @param data I dati da scrivere nel dispositivo
 * @return Lo status dell'operazione
 * */
static inline unsigned int write_to_flash(const int asid,
                                          const unsigned int data);

/**
 * @brief Aggiorna il TLB con le entryHI e entryLO passate. Esegue prima una
 * probe per controllare e' gia' presente un'entry con entryHi corrispondente
 * e successivamente una TLBWRI per aggiornarne i valori.
 * @return 1 se ha successo, 0 altrimenti
 * */
static inline int update_tlb(unsigned int entryHi, unsigned int entryLo);

inline void init_supp_structures(void)
{
  size_tt i, j;

  /* Inizializza il semaforo di mutua esclusione della swap pool a 1 */
  swp_pl_sem = 1;

  /* Imposta tutti i frame della swap pool come 'liberi' */
  for (i = 0; i < SWAP_POOL_SIZE; i++)
    swppl_tbl[i].asid = -1;

  /* Inizializza tutti i semafori dei device a 1 */
  for (i = 0; i < DEVINTNUM + 1; i++)
    for (j = 0; j < DEVPERINT; j++)
      dev_sems[i][j] = 1;
}

inline void tlb_exc_handler(void)
{
  support_t *const act_proc_sup =
      (support_t *const)SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if (act_proc_sup == NULL) {
    LOG("Error on getsupport");
    return;
  }

  const unsigned int cause =
      CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[PGFAULTEXCEPT].cause);

  if (cause == EXC_MOD) {
    LOG("EXECMOD");
    safe_kill();
    return;
  }

  /* P sul semaforo della swap pool */
  SYSCALL(PASSEREN, (unsigned int)&swp_pl_sem, 0, 0);

  size_tt mpg_no;

  /* Controllo se la pagina mancante è quella della stack */
  if ((mpg_no = ENTRYHI_GET_VPN(
           act_proc_sup->sup_exceptState[PGFAULTEXCEPT].entry_hi)) == STK_PG) {
    mpg_no = USERPGTBLSIZE - 1;
  }

  const unsigned int chosen_frame = chose_frame();
  swppl_entry_t *const ch_frame_entry = &swppl_tbl[chosen_frame];

  /* Guardo se il frame selezionato e' occupato */
  if (ch_frame_entry->asid != -1) {
    free_frame(ch_frame_entry, act_proc_sup->sup_asid);
  }

  /* Calcolo l'indirizzo di destinazione della swap pool */
  const unsigned int sp_addr =
      (unsigned int)SWAP_POOL_BEGIN + (chosen_frame * PAGESIZE);

  /* Leggo dal flash device */
  const unsigned int dev_stat =
      read_from_flash(act_proc_sup->sup_asid, mpg_no, sp_addr);

  /* Controllo che la lettura sia andata a buon fine */
  if (dev_stat != READY) {
    LOGi("error reading frame content from dev", dev_stat);
    return;
  }

  /* Disabilito gli interrupt per eseguire in modo atomico */
  toggle_int(FALSE);

  /* Aggiorno la swap pool table con le informaioni nuove */
  swppl_tbl[chosen_frame].asid = act_proc_sup->sup_asid;
  swppl_tbl[chosen_frame].vpn = KUSEG + (mpg_no << VPNSHIFT);
  swppl_tbl[chosen_frame].pg_tbl_entry =
      &act_proc_sup->sup_privatePgTbl[mpg_no];

  /* Aggiorno la page table del processo segnando la pagina valida e mettendo il
   * nuovo pfn */
  act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryLO =
      sp_addr | ENTRYLO_VALID | ENTRYLO_DIRTY;

  /* Aggiorno il TLB */
  update_tlb(act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryHI,
             act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryLO);

  /* Riabilito gli interrupt */
  toggle_int(TRUE);

  /* V nel semaforo della swap table per rilasciare la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&swp_pl_sem, 0, 0);

  LDST(&act_proc_sup->sup_exceptState[0]);
}

int update_tlb(unsigned int entryHi, unsigned int entryLo)
{
#define P_BIT 0x40000000
  unsigned int index;

  setENTRYHI(entryHi);
  TLBP();
  index = getINDEX();

  if (index & P_BIT) {
    LOG("TLB probe failed");
    return 0;
  }

  setENTRYLO(entryLo);
  TLBWI();
  return 1;
}

int chose_frame(void)
{
  static size_tt frame_top = 0;
  size_tt i;

  /* Partendo dall'ultimo frame scelto scorriamo la tabella della swap pool
   * in modo circolare finchè non ne troviamo uno libero.*/
  for (i = 0; i < SWAP_POOL_SIZE; i++) {
    if (swppl_tbl[(frame_top + i) % SWAP_POOL_SIZE].asid == -1) {
      return frame_top = (frame_top + i) % SWAP_POOL_SIZE;
    }
  }

  /* Se sono tutti occupati ritorniamo il frame successivo all'ultimo scelto */
  return frame_top = (frame_top + 1) % SWAP_POOL_SIZE;
}

void free_frame(swppl_entry_t *frame, const int asid)
{
  pteEntry_t *proc_pgtbl_entry;
  unsigned int dev_stat;

  /* Recupero la tabella delle pagine del processo */
  proc_pgtbl_entry = frame->pg_tbl_entry;

  /* Disabilito gli interrupt per eseguire in modo atomico */
  toggle_int(FALSE);

  /* Marco la pagina come invalida */
  if (proc_pgtbl_entry->pte_entryLO & ENTRYLO_VALID)
    proc_pgtbl_entry->pte_entryLO ^= ENTRYLO_VALID;

  /* Aggiorno il TLB */
  update_tlb(proc_pgtbl_entry->pte_entryHI, proc_pgtbl_entry->pte_entryLO);

  /* Riabilito gli interrupt */
  toggle_int(TRUE);

  /* Scrivo nel dispostivio flash i dati presenti al pfn */
  dev_stat =
      write_to_flash(asid, ENTRYLO_GET_PFN(proc_pgtbl_entry->pte_entryLO));

  /* Controllo che la DO IO sia andata a buon fine */
  if (dev_stat != READY) {
    LOGi("error writing frame to dev", dev_stat);
    return;
  }

  /* Marco il frame come libero */
  frame->asid = -1;
}

void toggle_int(int on)
{
  if (on) {
    setSTATUS(getSTATUS() | 1);
  } else {
    setSTATUS((getSTATUS() | 1) ^ 1);
  }
}

unsigned int read_from_flash(const int asid, const unsigned int pg_no,
                             const unsigned int dest)
{
  dtpreg_t *dev_reg;
  unsigned int cmdval;
  unsigned int dev_stat;

  /* Guadagno l'accesso al device in mutua esclusione */
  SYSCALL(PASSEREN, (unsigned int)&dev_sems[FLASH_SEMS][asid - 1], 0, 0);

  /* Prendo il device register del dispositivo flash associato all'asid */
  dev_reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);

  /* Scrivi il contenuto di data0 nella destinazione scelta */
  dev_reg->data0 = dest;

  /* Imposto il command */
  cmdval = (pg_no << 8) | FLASHREAD;

  /* Uso la NSYS5 per dire al controller del device di leggere */
  dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, cmdval, 0);

  /* Rilascio la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&dev_sems[FLASH_SEMS][asid - 1], 0, 0);

  return dev_stat;
}

unsigned int write_to_flash(const int asid, const unsigned int data)
{
  unsigned int dev_stat;
  dtpreg_t *dev_reg;

  /* Guadagno l'accesso al device in mutua esclusione */
  SYSCALL(PASSEREN, (unsigned int)&dev_sems[FLASH_SEMS][asid - 1], 0, 0);

  /* Prendo i registri del device */
  dev_reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);

  /* Metto in data0 ciò che voglio scrivere */
  dev_reg->data0 = data;

  /* Scrivo su command il comando per scrivere (lol)*/
  dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, FLASHWRITE, 0);

  /* Rilascio la mutua esclusione */
  SYSCALL(VERHOGEN, (unsigned int)&dev_sems[FLASH_SEMS][asid - 1], 0, 0);

  return dev_stat;
}