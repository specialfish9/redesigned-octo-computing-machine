#include "interrupts.h"
#include "asl.h"
#include "exceptions.h"
#include "klog.h"
#include "listx.h"
#include "pandos_const.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define TERMSTATMASK 0xFF
#define RECVD 5
#define PLTINT 1
#define ITINT 2

int sem_it;
int sem_disk[DEVPERINT];
int sem_flash[DEVPERINT];
int sem_net[DEVPERINT];
int sem_printer[DEVPERINT];
int sem_term_in[DEVPERINT];
int sem_term_out[DEVPERINT];

/** TODO add doc*/
static inline void generic_interrupt_handler(int line, int *semaphores);



inline void init_dev_sem(void)
{
  size_tt i;

  sem_it = 0;
  for (i = 0; i < DEVPERINT; ++i)
    sem_disk[i] = sem_flash[i] = sem_net[i] = sem_printer[i] = sem_term_in[i] =
        sem_term_out[i] = 0;
}

inline void handle_interrupts(const int line)
{
  kprint("INT");
  kprint_int(line);
  kprint("|");

  switch (line) {
  case 0: {
    break; /* safely ignore */
  }
  case PLTINT: {
    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
    break;
  }
  case ITINT: {
    /* Pseudo-clock Tick */
    LDIT(PSECOND);
    pcb_t *p;
    while ((p = remove_blocked(&sem_it)) != NULL)
      enqueue_proc(p, p->p_prio);
    sem_it = 0;
    break;
  }
  case DISKINT: {
    generic_interrupt_handler(IL_DISK - IL_DISK, sem_disk);
    break;
  }
  case FLASHINT: {
    generic_interrupt_handler(IL_FLASH - IL_DISK, sem_disk);
    break;
  }
  case NETWINT: {
    generic_interrupt_handler(IL_ETHERNET - IL_DISK, sem_disk);
    break;
  }
  case PRNTINT: { /* TODO */
    generic_interrupt_handler(IL_PRINTER - IL_DISK, sem_disk);
    break;
  }
  case TERMINT: {
    /* todo check order, maybe transm and recv should be swapped */
    int *sem[] = {sem_term_in, sem_term_out};
    size_tt bitmap = CDEV_BITMAP_ADDR(line), index = 0;
    while (bitmap > 1) {
      ++index;
      bitmap >>= 1;
    }

    termreg_t *reg = (termreg_t *)&((devregarea_t *)RAMBASEADDR)
                         ->devreg[IL_TERMINAL - IL_DISK][index]
                         .term;
    /* todo check order, maybe transm and recv should be swapped */
    size_tt status[2] = {reg->transm_status, reg->recv_status};
    size_tt *command[2] = {&reg->transm_command, &reg->recv_command};
    for (int i = 0; i < 2; ++i) {
      pcb_t *p = verhogen(&sem[i][index]);
      if (p != NULL)
        p->p_s.reg_v0 = status[i];

      /* send ack */
      *command[i] = 1;
    }
    break;
  }
  default:
    break;
  }
}

static inline void generic_interrupt_handler(int line, int *semaphores)
{
  size_tt bitmap = CDEV_BITMAP_ADDR(line), index = 0;
  while (bitmap > 1) {
    ++index;
    bitmap >>= 1;
  }
  dtpreg_t *reg =
      (dtpreg_t *)&((devregarea_t *)RAMBASEADDR)->devreg[line][index].dtp;
  pcb_t *p = verhogen(semaphores + index);
  if (p != NULL)
    p->p_s.reg_v0 = reg->status;
  /* send ack */
  reg->command = 1;
}
