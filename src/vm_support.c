#include "vm_support.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "syscalls.h"
#include "utils.h"
#include "umps3/umps/cp0.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>

#define SWAP_POOL_SIZE 2 * UPROCMAX 
#define SWAP_POOL_BEGIN 0x20020000
#define LOG(s) log("VM", s)
#define LOGi(s, i) logi("VM", s, i)

typedef struct {
  int asid;
  int vpn;
  pteEntry_t *pg_tbl_entry;
} swppl_entry_t;

/* TODO static */
int swp_pl_sem = 1; 
/* TODO static */
swppl_entry_t swppl_tbl[SWAP_POOL_SIZE];

/*
 * @var Putatore al frame da scegliere quando si esegue l'algoritmo di 
 * rimpiazzamento
 * */
static size_tt frm_ch_ptr = 0;

/* TODO static */
int chose_frame(void);
static int write_to_dev(void);

inline void init_supp_structures(void) {
  size_tt i;

  /* Imposta tutti i frame della swap pool come 'liberi' */
  for (i = 0; i < SWAP_POOL_SIZE; i++)
    swppl_tbl[i].asid = -1;

  /* TODO init shared device sems */
}

inline void tlb_exc_handler(void) 
{
  support_t *act_proc_sup; 
  unsigned int cause;
  unsigned int missing_pg;
  state_t *saved_state;
  dtpreg_t *dev_reg;
  swppl_entry_t *ch_frame_entry;
  unsigned int chosen_frame;
  int dev_stat;

  act_proc_sup = (support_t*) SYSCALL(GETSUPPORTPTR, 0, 0, 0);
  if (act_proc_sup == NULL) {
    LOG("Error on getsupport");
    return;
    /* TODO handle error */
  }
  
  cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[0].cause); 
  LOGi("tlbeh", cause);

  if (cause == EXC_MOD) {
    /* Tentativo di accesso ad una pagina read-only - trap */
    /* TODO trap */
  } else {

    /* P sul semaforo della swap pool */
    SYSCALL(PASSEREN, (unsigned int) &swp_pl_sem, 0, 0);

    /* Recupero lo stato salvato dell'eccezione */
    saved_state = (state_t *)BIOSDATAPAGE;

    missing_pg = saved_state->entry_hi >> VPNSHIFT;
    LOGi("Missing pg:", missing_pg);
    
    ch_frame_entry = &swppl_tbl[(chosen_frame = chose_frame())];

    LOGi("Chosen frame", chosen_frame);

    /* Guardo se il frame selezionato e' occupato */
    if (ch_frame_entry->asid != -1) {
      LOG("occupato");
      /* Recupero la tabella delle pagine del processo */
      pteEntry_t *proc_pgtbl_entry = ch_frame_entry->pg_tbl_entry;
      /* Marco la pagina come invalida */
      proc_pgtbl_entry->pte_entryLO &= 0 << ENTRYLO_VALID_BIT; 

      /* Aggiorno il TLB */
      /* TODO il manuale consiglia di iniziare facendo clear completo */
      TLBCLR();

      /* SUPER- TODO:
       * Se questa cosa funziona ricordarsi di accendere un cero per ogni riga
       * di codice scritta in questo progetto. */

      /* Prendo i registri del device */
      dev_reg = (dtpreg_t*) DEV_REG_ADDR(3, act_proc_sup->sup_asid);

      /* Metto in data0 il pfn che voglio scrivere */
      dev_reg->data0 = ENTRYLO_GET_PFN(proc_pgtbl_entry->pte_entryLO);

      /* Scrivo su command il comando per scrivere (lol)*/
      if ((dev_stat = SYSCALL(DOIO, (int) DEV_REG_ADDR(FLASHINT, act_proc_sup->sup_asid), FLASHWRITE, 0)) != 1) {
        LOGi("error writing frame to dev", dev_stat);
        /* TODO trap */
        return;
      } 
    }    
    LOG("Libero");

    /* Prendo il device register */
    dev_reg = (dtpreg_t*) DEV_REG_ADDR(3, act_proc_sup->sup_asid);

    /* Uso la NSYS5 per dire al controller del device di leggere */
    if ((dev_stat = SYSCALL(DOIO, (int) DEV_REG_ADDR(FLASHINT, act_proc_sup->sup_asid), FLASHREAD, 0)) != 1) {
      LOGi("error reading frame content from dev", dev_stat);
      /* TODO trap */
      return;
    } 

    /* Scrivi il contenuto di data0 sul frame della swap pool scelto */
    memcpy((memaddr*) SWAP_POOL_BEGIN + (chosen_frame * PAGESIZE), &dev_reg->data0, sizeof(dev_reg->data0));

    /* Aggiorno la swap pool table con le informaioni nuove */
    /*TODO e' giusto?*/
    swppl_tbl[chosen_frame].asid = act_proc_sup->sup_asid;
    swppl_tbl[chosen_frame].vpn= missing_pg;
    swppl_tbl[chosen_frame].pg_tbl_entry = &act_proc_sup->sup_privatePgTbl[missing_pg];

    /* TODO use setSTATUS per fare le cose in modo atomico (anche sopra)*/
    /* Aggiorno la page table del processo segnando la pagina valida */
    act_proc_sup->sup_privatePgTbl[missing_pg].pte_entryLO |= ENTRYLO_VALID;
    /* e mettendo il nuovo pfn */
    /* TODO trovare assolutamente un modo migliore di farlo */
    int tmp = SWAP_POOL_BEGIN + (chosen_frame * PAGESIZE);
    tmp <<= ENTRYLO_PFN_BIT;
    tmp |= ((act_proc_sup->sup_privatePgTbl[missing_pg].pte_entryLO << ENTRYLO_PFN_BIT) >> ENTRYLO_PFN_BIT);
    act_proc_sup->sup_privatePgTbl[missing_pg].pte_entryLO = tmp;
    
    /*TODO update the TLB*/
    TLBCLR();

    /* V nel semaforo della swap table per rilasciare la mutua esclusione */
    SYSCALL(VERHOGEN, (unsigned int) &swp_pl_sem, 0, 0);


    /* SUPER-TODO capire come fare un ldst in modo safe per non mandare a puttane
     * lo scheduler */
    LOG("DOne");
    LDST(&act_proc_sup->sup_exceptState[0]);
  }


}

inline int chose_frame(void){ return frm_ch_ptr++ % SWAP_POOL_SIZE; }

inline int write_to_dev() {
  return TRUE;
}
