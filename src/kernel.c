#include "asl.h"
#include "exceptions.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"
#include "pcb.h"
#include "scheduler.h"
#include "term_utils.h"
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define DEV_NUM 10 /* TODO */

static int dev_sem[DEV_NUM];
static passupvector_t *passup_vec;

inline static void init_passup_vector(void);
inline static void init_data_structures(void);
inline static void init_devices(void);

inline static void uTLB_RefillHandler(void);
inline static void exception_handler(void);

extern void test();

int main(void)
{
  print1("Init passup vector...");
  init_passup_vector();
  print1("done!\n");
  kprint("Init pv done");

  print1("Init data structures...");
  init_data_structures();
  print1("done!\n");
  kprint("Init data str done");

  print1("Loading interval timer...");
  LDIT(100000); /* 100 millisecs */
  print1("done!\n");
  kprint("IT load done");

  print1("Init devices...");
  init_devices();
  print1("done!\n");

  print1("Creating init process...");
  create_init_proc((memaddr)test);
  print1("done!\n");
  kprint("Init proc done");

  print1("Starting init process...\n");
  scheduler_next();

  return 0;
}

void init_passup_vector(void)
{
  passup_vec = (passupvector_t *)PASSUPVECTOR;
  passup_vec->tlb_refill_handler = (memaddr)uTLB_RefillHandler;
  passup_vec->exception_handler = (memaddr)exception_handler;
  passup_vec->tlb_refill_stackPtr = KERNELSTACK;
  passup_vec->exception_stackPtr = KERNELSTACK;
}

void init_data_structures(void)
{
  size_tt i;

  init_pcbs();
  init_asl();

  init_scheduler();

  i = 0;
  while (i < DEV_NUM)
    dev_sem[i++] = 0;
}

void init_devices(void)
{
  size_tt i = 0;

  while (i++ < DEV_NUM) {
    dev_sem[i] = 0;
  }
}

void exception_handler(void)
{ /* place holder */
  print1("EXCEPTION HANDLER FIRED");
  kprint("exc handl");
  /*
Per distinguere effettivamente di che eccezione si
tratta bisogna leggere il registro Cause.ExcCode:
- 0 = Interrupt
- 1-3 = TLB Trap
- 4-7,9-12 = Program Trap
- 8 = Syscall

per TLB trap e PROGRAM trap passa il controllo a support struct del processo o
ammaizzalo
Any attempt to request one of these services
while in user-mode should trigger a Program Trap exception respons
   */
}
/* TLB-Refill Handler */
/* One can place debug calls here, but not calls to print */
void uTLB_RefillHandler()
{
  print1("TLB refill called");
  kprint("TLB refill called");
  setENTRYHI(0x80000000);
  setENTRYLO(0x00000000);
  TLBWR();

  LDST((state_t *)0x0FFFF000);
}

void handle_syscall(int number, unsigned int arg1, unsigned int arg2,
                    unsigned int arg3)
{

  switch (number) {
  case CREATEPROCESS: {
    int status;
    status = create_process((state_t *)arg1, (int)arg2, (support_t *)arg3);
    /* Errore */
    if (status < 0) {
    } else {
      /* Successo */
    }
    break;
  }
  case PASSEREN: {
    // passeren(arg1); //forse va un puntatore
    break;
  }
  case VERHOGEN: {
    // verhogen(arg1); //forse va un puntatore
    break;
  }
  case CLOCKWAIT: {
    // int tmp = wait_for_clock();
    break;
  }
  default:
    /* TODO Any
attempt to request a non-existent Nucleus service should trigger a Program
Trap exception too*/
    break;
  }
}
