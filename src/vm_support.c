#include "vm_support.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "syscalls.h"
#include "utils.h"
#include "umps3/umps/cp0.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/arch.h>

#define IDK 42 /* Todo size of swppl table */
#define LOG(s) log("VMS", s)
#define LOGi(s, i) logi("VMS", s, i)

typedef struct {
  int asid;
  int vpn;
  pteEntry_t *pg_tbl_entry;
} swppl_entry_t;

/* TODO static */
int swp_pl_sem = 1; 
/* TODO static */
swppl_entry_t swppl_tbl[IDK];

/* TODO static */
int chose_frame(void);
static int write_to_dev(void);

inline void init_supp_structures(void) {
  /* TODO init swap pool */
  /* TODO init shared device sems */
}

inline void tlb_exc_handler(void) 
{
  support_t *act_proc_sup; 
  unsigned int cause;
  unsigned int missing_pg;
  state_t *saved_state;
  swppl_entry_t *selected_frame;
  dtpreg_t *dev_reg;

  LOG("Ora xe cassi vostri");

  if (SYSCALL(GETSUPPORTPTR, 0, 0, 0) < 0) {
    LOG("Error on getsupport");
    return;
    /* TODO handle error */
  }
  
  cause = CAUSE_GET_EXCCODE(act_proc_sup->sup_exceptState[0].cause); 

  if (cause == EXC_MOD) {
    /* Tentativo di accesso ad una pagina read-only - trap */
    /* TODO trap */
  } else {

    /* P sul semaforo della swap pool */
    passeren(&swp_pl_sem);

    /* Recupero lo stato salvato dell'eccezione */
    saved_state = (state_t *)BIOSDATAPAGE;

    missing_pg = ENTRYHI_GET_VPN(saved_state->entry_hi);
    LOGi("Missing pg:\n", missing_pg);
    

    selected_frame = &swppl_tbl[chose_frame()];

    /* Guardo se il frame selezionato e' occupato */
    if (selected_frame->asid != -1) {
      pteEntry_t *proc_pgtbl_entry = selected_frame->pg_tbl_entry;
      proc_pgtbl_entry->pte_entryLO |=  ENTRYLO_VALID; /* TODO check if the bit must be on or off */

      /* Aggiorno il TLB */
      /* TODO il manuale consiglia di iniziare facendo clear completo */
      TLBCLR();

      /* SUPER- TODO:
       * Se questa cosa funziona ricordarsi di accendere un cero per ogni riga
       * di codice scritta in questo progetto. */
      dev_reg = (dtpreg_t *)&((devregarea_t *)RAMBASEADDR)->devreg[FLASHINT][selected_frame->asid].dtp; 

      dev_reg->data0 = ENTRYLO_GET_PFN(proc_pgtbl_entry->pte_entryLO);
      dev_reg->command = 0 | FLASHWRITE;

      /*TODO Update process x’s backing store. Write the contents of frame i
      to the correct location on process x’s backing store/flash device.
      [Section 4.5.1]
      Treat any error status from the write operation as a program trap.
      [Section 4.8]*/

      /* TODO punto 9*/
    }    

  }
}

inline int chose_frame(void){ return 42; }

inline int write_to_dev() {
  return TRUE;
}
