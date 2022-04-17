#include "interrupts.h"
#include "asl.h"
#include "exceptions.h"
#include "klog.h"
#include "listx.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOGi(s, i)                                                             \
  kprint("I>" s);                                                              \
  kprint_int(i);                                                               \
  kprint("|")

#define LOG(s) kprint("I>" s)

#define TERMSTATMASK 0xFF
#define RECVD 5

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
  LOGi("INT", line);

  switch (line) {
  case IL_IPI: {
    break; /* safely ignore */
  }
  case IL_CPUTIMER: { /* PLT */
    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
    break;
  }
  case IL_TIMER: {
    /* Pseudo-clock Tick */
    LDIT(PSECOND);
    pcb_t *p;
    while ((p = remove_blocked(&sem_it)) != NULL)
      enqueue_proc(p, p->p_prio);
    sem_it = 0;
    break;
  }
  case IL_DISK: {
    generic_interrupt_handler(IL_DISK - IL_DISK, sem_disk);
    break;
  }
  case IL_FLASH: {
    generic_interrupt_handler(IL_FLASH - IL_DISK, sem_disk);
    break;
  }
  case IL_ETHERNET: {
    generic_interrupt_handler(IL_ETHERNET - IL_DISK, sem_disk);
    break;
  }
  case IL_PRINTER: { /* TODO */
    generic_interrupt_handler(IL_PRINTER - IL_DISK, sem_disk);
    break;
  }
  case IL_TERMINAL: {
    int *sem[] = {sem_term_out, sem_term_in};
    size_tt *bitmap = (size_tt *)CDEV_BITMAP_ADDR(line), index = 0;
    while (*bitmap > 1) {
      ++index;
      *bitmap >>= 1;
    }

    termreg_t *reg = (termreg_t *)&((devregarea_t *)RAMBASEADDR)
                         ->devreg[IL_TERMINAL - IL_DISK][index]
                         .term;

    /* todo check order, maybe transm and recv should be swapped */
    size_tt status[2] = {reg->transm_status, reg->recv_status};
    size_tt *command[2] = {&reg->transm_command, &reg->recv_command};
    for (int i = 0; i < 2; ++i) {
      pcb_t *p = verhogen(&sem[i][index]);
      if (p != NULL) {
        p->p_s.reg_v0 = status[i];
      }
      /* send ack */
      *command[i] = 1;
    }
    LOG("END");
    break;
  }
  default:
    break;
  }
}

static inline void generic_interrupt_handler(int line, int *semaphores)
{
  size_tt bitmap, index;
  dtpreg_t *reg;
  pcb_t *p;

  bitmap = CDEV_BITMAP_ADDR(line), index = 0;
  while (bitmap > 1) {
    ++index;
    bitmap >>= 1;
  }
  reg = (dtpreg_t *)&((devregarea_t *)RAMBASEADDR)->devreg[line][index].dtp;
  p = verhogen(semaphores + index);
  if (p != NULL)
    p->p_s.reg_v0 = reg->status;
  /* send ack */
  reg->command = 1;
}
