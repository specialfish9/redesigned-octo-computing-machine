#include "vm_support.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "syscalls.h"
#include "umps3/umps/cp0.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>

#define SWAP_POOL_SIZE 2 * UPROCMAX
#define SWAP_POOL_BEGIN 0x20020000

#define CALC_NEW_PFN(entryLO, pfn) (pfn << ENTRYLO_PFN_BIT) | (entryLO & 2047)
#define PAGE_N(pgaddr) (pgaddr - 0x80000) / PAGESIZE

#define LOG(s) log("VM", s)
#define LOGi(s, i) logi("VM", s, i)

typedef struct {
  int asid;
  int vpn;
  pteEntry_t *pg_tbl_entry;
} swppl_entry_t;

static int swp_pl_sem = 1;
static swppl_entry_t swppl_tbl[SWAP_POOL_SIZE];

/*
 * @var Putatore al frame da scegliere quando si esegue l'algoritmo di
 * rimpiazzamento
 * */
static size_tt frm_ch_ptr = 0;

static int chose_frame(void);

static void toggle_int(int on);

inline void init_supp_structures(void)
{
  size_tt i;

  /* Imposta tutti i frame della swap pool come 'liberi' */
  for (i = 0; i < SWAP_POOL_SIZE; i++)
    swppl_tbl[i].asid = -1;

  /* TODO init shared device sems */
}

inline void tlb_exc_handler(void)
{
  support_t *act_proc_sup = (support_t *)SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  if (act_proc_sup == NULL) {
    LOG("Error on getsupport");
    return;
    /* TODO handle error */
  }

  unsigned int cause =
      CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[PGFAULTEXCEPT].cause);
  LOGi("tlbeh", cause);

  if (cause == EXC_MOD) {
    LOG("EXECMOD");
    /* Tentativo di accesso ad una pagina read-only - trap */
    /* TODO trap */
  } else {

    /* P sul semaforo della swap pool */
    SYSCALL(PASSEREN, (unsigned int)&swp_pl_sem, 0, 0);

    unsigned int chosen_frame = chose_frame();
    swppl_entry_t *ch_frame_entry = &swppl_tbl[chosen_frame];

    LOGi("Chosen frame", chosen_frame);

    /* Guardo se il frame selezionato e' occupato */
    if (ch_frame_entry->asid != -1) {
      LOG("occupato");
      /* Recupero la tabella delle pagine del processo */
      pteEntry_t *proc_pgtbl_entry = ch_frame_entry->pg_tbl_entry;

      /* Disabilito gli interrupt per eseguire in modo atomico */
      toggle_int(0);

      /* Marco la pagina come invalida */
      proc_pgtbl_entry->pte_entryLO &= 0 << ENTRYLO_VALID_BIT;

      /* Aggiorno il TLB */
      /* TODO il manuale consiglia di iniziare facendo clear completo */
      TLBCLR();

      /* Riabilito gli interrupt */
      toggle_int(1);

      /* SUPER- TODO:
       * Se questa cosa funziona ricordarsi di accendere un cero per ogni riga
       * di codice scritta in questo progetto. */

      /* Prendo i registri del device */
      dtpreg_t *dev_reg =
          (dtpreg_t *)DEV_REG_ADDR(FLASHINT, act_proc_sup->sup_asid);

      /* Metto in data0 il pfn che voglio scrivere */
      dev_reg->data0 = ENTRYLO_GET_PFN(proc_pgtbl_entry->pte_entryLO);

      /* Scrivo su command il comando per scrivere (lol)*/
      unsigned int dev_stat;
      /* TODO controlalre che effettivamente il command register venga impostato
       * correttamente */
      if ((dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, FLASHWRITE,
                              0)) != READY) {
        LOGi("error writing frame to dev", dev_stat);
        /* TODO trap */
        return;
      }
    }

    size_tt missing_pg = act_proc_sup->sup_exceptState[PGFAULTEXCEPT].entry_hi >> VPNSHIFT;
    size_tt mpg_no = PAGE_N(missing_pg);

    /* Prendo il device register del dispositivo flash associato al processo */
    dtpreg_t *dev_reg =
        (dtpreg_t *)DEV_REG_ADDR(FLASHINT, act_proc_sup->sup_asid);

    /* Scrivi il contenuto di data0 sul frame della swap pool scelto */
    dev_reg->data0 = (unsigned int)SWAP_POOL_BEGIN + (chosen_frame * PAGESIZE);

    /* Imposto il command */
    unsigned int cmdval = (mpg_no << 8) | FLASHREAD;

    /* Uso la NSYS5 per dire al controller del device di leggere */
    unsigned int dev_stat;
    if ((dev_stat = SYSCALL(DOIO, (unsigned int)&dev_reg->command, cmdval,
                            0)) != READY) {
      LOGi("error reading frame content from dev", dev_stat);
      /* TODO trap */
      return;
    }

    /* Aggiorno la swap pool table con le informaioni nuove */
    swppl_tbl[chosen_frame].asid = act_proc_sup->sup_asid;
    swppl_tbl[chosen_frame].vpn = missing_pg;
    swppl_tbl[chosen_frame].pg_tbl_entry =
        &act_proc_sup->sup_privatePgTbl[mpg_no];

    /* Disabilito gli interrupt per eseguire in modo atomico */
    toggle_int(0);

    /* Aggiorno la page table del processo segnando la pagina valida */
    act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryLO |= ENTRYLO_VALID;

    /* e mettendo il nuovo pfn */
    /* TODO pray and spray */
    act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryLO = CALC_NEW_PFN(
        act_proc_sup->sup_privatePgTbl[mpg_no].pte_entryLO, chosen_frame);

    /* TODO update the TLB*/
    TLBCLR();
    /* riabilito gli interrupt */
    toggle_int(1);

    /* V nel semaforo della swap table per rilasciare la mutua esclusione */
    SYSCALL(VERHOGEN, (unsigned int)&swp_pl_sem, 0, 0);

    /* SUPER-TODO capire come fare un ldst in modo safe per non mandare a
     * puttane lo scheduler */
    LDST(&act_proc_sup->sup_exceptState[0]);
  }
}

inline int chose_frame(void) { return frm_ch_ptr++ % SWAP_POOL_SIZE; }

inline void toggle_int(int on) {
  if (on) {
    setSTATUS(getSTATUS() | 1);
  } else {
    setSTATUS((getSTATUS() >> 1) << 1);
  }
}
