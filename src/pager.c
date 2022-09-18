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
#include "support.h"
#include "sys_support.h"
#include "syscalls.h"
#include "umps3/umps/cp0.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/libumps.h>

/**
 * @brief Indirizzo di inizio della Swap Pool
 * */
#define SP_BEGIN 0x20020000

/* Macro per il log */
#define LOG(s) log("VM", s)
#define LOGi(s, i) logi("VM", s, i)

/**
 * @var Array con le entry della swap pool
 * */
sp_entry_t sp_tbl[SP_SIZE];

/**
 * @var Semaforo mutex per la swap pool
 * */
int sp_sem;

/**
 * @var Variabile per tenere conto dei processi i cui frame che occupano la swap
 * pool sono utilizzabili. In particolare essendo una variabile di tipo char e' 
 * composta da 8 bit. Se il bit i-esimo e' a 1 significa che la swap pool contiene
 * frame del processo con asid i + 1 e che questo processo e' vivo. Se il bit
 * i-esimo e' 0 allora il processo e' morto e/o la swap pool non contiene frame
 * di sua proprieta'.
 * */
char sp_asids;

/**
 * @brief Riempie un frame della swap pool
 * @param sup Struttura di supporto del processo proprietario del frame da riempire
 * @param frame Numero del frame da riempire
 * @param pg_no Numero della pagina da scrivere sul frame
 * */
static inline void fill_frame(support_t *const sup, const size_tt frame, const size_tt pg_no);

/**
 * @brief Libera un frame salvandone il contenuto nel dispositivo flash
 * @param frame puntatore al frame da liberare
 * @param asid l'asid del processo proprietario del frame
 * */
static inline void free_frame(sp_entry_t *frame, const int asid);

/**
 * @brief Utilizza sp_asids per marcare i frame del processo specificato come 
 * utilizzabili
 * @param asid L'asid del processo coinvolto
 * */
void clean_frames(const unsigned int asid);

/**
 * @brief Implementa l'algoritmo di rimpiazzamento per i frame nella swap pool
 * @return Il numero del frame da rimpiazzare
 * */
static inline int choose_frame(void);

/**
 * @brief Utility per modificare il valore della variabile @ref sp_asids
 * @param on Booleano. Se 1 il bit del processo verra' acceso. Se e' 0 verra'
 * spento.
 * @param asid L'asid del processo coinvolto.
 * */
static inline void set_sp_asids(const int on, const unsigned int asid);

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
static inline unsigned int read_from_flash(const unsigned int asid,
                                           const unsigned int pg_no,
                                           const unsigned int dest);

/**
 * @brief Funzione per scrivere su un dispositivo flash
 * @param asid L'asid del processo
 * @param data I dati da scrivere nel dispositivo
 * @return Lo status dell'operazione
 * */
static inline unsigned int write_to_flash(const unsigned int asid,
                                          const unsigned int blk_no, 
                                          const unsigned int data);

/**
 * @brief Aggiorna il TLB con le entryHI e entryLO passate. Esegue prima una
 * probe per controllare e' gia' presente un'entry con entryHi corrispondente
 * e successivamente una TLBWRI per aggiornarne i valori.
 * @return 1 se ha successo, 0 altrimenti
 * */
static inline int update_tlb(unsigned int entryHi, unsigned int entryLo);


inline void tlb_exc_handler(void)
{
  unsigned int cause;
  unsigned int sp_addr;
  unsigned int dev_stat;
  unsigned int chosen_frame;
  size_tt mpg_no;
  support_t *act_proc_sup;
  sp_entry_t *ch_frame_entry;

  act_proc_sup = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);

  if (act_proc_sup == NULL) {
    LOG("Error on getsupport");
    return;
  }

  cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[PGFAULTEXCEPT].cause);

  if (cause == EXC_MOD) {
    LOG("EXECMOD");
    safe_kill();
    return;
  }

  /* P sul semaforo della swap pool */
  SYSCALL(PASSEREN, (unsigned int)&sp_sem, 0, 0);

  /* Controllo se la pagina mancante è quella della stack */
  mpg_no = ENTRYHI_GET_VPN(act_proc_sup->sup_exceptState[PGFAULTEXCEPT].entry_hi);
  mpg_no = mpg_no == STK_PG? USERPGTBLSIZE - 1 : mpg_no;

  chosen_frame = choose_frame();
  ch_frame_entry = &sp_tbl[chosen_frame];

  /* Guardo se il frame selezionato e' utilizzabile */
  if (ch_frame_entry->asid != -1 && (sp_asids & (1 << (ch_frame_entry->asid - 1)))) {
    free_frame(ch_frame_entry, act_proc_sup->sup_asid);
  }

  /* Calcolo l'indirizzo di destinazione della swap pool */
  sp_addr = (unsigned int)SP_BEGIN + (chosen_frame * PAGESIZE);

  /* Leggo dal flash device */
  dev_stat = read_from_flash(act_proc_sup->sup_asid, mpg_no, sp_addr);

  /* Controllo che la lettura sia andata a buon fine */
  if (dev_stat != READY) {
    LOGi("error reading frame content from dev", dev_stat);
    return;
  }

  /* Disabilito gli interrupt per eseguire in modo atomico */
  toggle_int(FALSE);

  /* Aggiorno la swap pool table con le informaioni nuove */
  fill_frame(act_proc_sup, chosen_frame, mpg_no);

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
  SYSCALL(VERHOGEN, (unsigned int)&sp_sem, 0, 0);

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

int choose_frame(void)
{
  static size_tt frame_top = 0;
  size_tt i;

  /* Partendo dall'ultimo frame scelto scorriamo la tabella della swap pool
   * in modo circolare finchè non ne troviamo uno libero.*/
  for (i = 0; i < SP_SIZE; i++) {
    if (sp_tbl[(frame_top + i) % SP_SIZE].asid == -1) {
      return frame_top = (frame_top + i) % SP_SIZE;
    }
  }

  /* Se sono tutti occupati ritorniamo il frame successivo all'ultimo scelto */
  return frame_top = (frame_top + 1) % SP_SIZE;
}

void fill_frame(support_t *const sup, const size_tt frame, const size_tt pg_no)
{
  if (frame < 0 || frame >= SP_SIZE) {
    LOGi("Invalid frame number", frame);
    PANIC();
  }

  sp_tbl[frame].asid = sup->sup_asid;
  sp_tbl[frame].vpn = KUSEG + (pg_no << VPNSHIFT);
  sp_tbl[frame].pg_tbl_entry = &sup->sup_privatePgTbl[pg_no];

  set_sp_asids(TRUE, sup->sup_asid);
}

void free_frame(sp_entry_t *frame, const int asid)
{
  pteEntry_t *proc_pgtbl_entry;
  unsigned int dev_stat;
  unsigned int vpn;

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
  
  /* Recupero il numero della pagina da scrivere */
  vpn = ENTRYHI_GET_VPN(proc_pgtbl_entry->pte_entryHI);
  vpn = vpn == STK_PG? USERPGTBLSIZE - 1 : vpn;

  /* Scrivo nel dispostivio flash i dati presenti al pfn */
  dev_stat =
      write_to_flash(asid, vpn, ENTRYLO_GET_PFN(proc_pgtbl_entry->pte_entryLO));

  /* Controllo che la DO IO sia andata a buon fine */
  if (dev_stat != READY) {
    LOGi("error writing frame to dev", dev_stat);
    return;
  }

  /* Marco il frame come libero */
  frame->asid = -1;
}

void clean_frames(const unsigned int asid)
{
  set_sp_asids(FALSE, asid);
}

void set_sp_asids(const int on, const unsigned int asid)
{
  if (asid > 8 || asid <= 0) {
    LOGi("Invalid asid on sp_asids:", asid);
    PANIC();
  }

  if (on)
    sp_asids |= (1 << (asid - 1));
  else 
    sp_asids ^= (1 << (asid - 1));
}

void toggle_int(int on)
{
  if (on) {
    setSTATUS(getSTATUS() | 1);
  } else {
    setSTATUS((getSTATUS() | 1) ^ 1);
  }
}

unsigned int read_from_flash(const unsigned int asid, const unsigned int pg_no,
                             const unsigned int dest)
{
  dtpreg_t *dev_reg;
  unsigned int cmdval;
  unsigned int dev_stat;

  /* Guadagno l'accesso al device in mutua esclusione */
  p_on_dev(FLASH_SEMS, asid - 1);

  /* Prendo il device register del dispositivo flash associato all'asid */
  dev_reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);

  /* Scrivi il contenuto di data0 nella destinazione scelta */
  dev_reg->data0 = dest;

  /* Imposto il command */
  cmdval = (pg_no << 8) | FLASHREAD;

  /* Uso la NSYS5 per dire al controller del device di leggere */
  dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, cmdval, 0);

  /* Rilascio la mutua esclusione */
  v_on_dev(FLASH_SEMS, asid - 1);

  return dev_stat;
}

unsigned int write_to_flash(const unsigned int asid, const unsigned int blk_no, const unsigned int data)
{
  unsigned int dev_stat;
  unsigned int cmdval;
  dtpreg_t *dev_reg;

  /* Guadagno l'accesso al device in mutua esclusione */
  p_on_dev(FLASH_SEMS, asid - 1);

  /* Prendo i registri del device */
  dev_reg = (dtpreg_t *)DEV_REG_ADDR(FLASHINT, asid - 1);

  /* Metto in data0 ciò che voglio scrivere */
  dev_reg->data0 = data;

  /* Imposto il command */
  cmdval = (blk_no << 8) | FLASHWRITE;

  /* Scrivo su command il comando per scrivere (lol)*/
  dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, cmdval, 0);

  /* Rilascio la mutua esclusione */
  v_on_dev(FLASH_SEMS, asid - 1);

  return dev_stat;
}
