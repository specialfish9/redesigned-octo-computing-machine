#include "interrupts.h"
#include "asl.h"
#include "syscalls.h"
#include "kernel.h"
#include "klog.h"
#include "listx.h"
#include "scheduler.h"
#include "utils.h"
#include <umps3/umps/arch.h>
#include <umps3/umps/const.h>
#include <umps3/umps/cp0.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

#define LOG(s) log("I", s)
#define LOGi(s, i) logi("I", s, i)

#define TERMSTATMASK 0xFF
#define RECVD 5

int sem_it;
int sem_disk[DEVPERINT];
int sem_flash[DEVPERINT];
int sem_net[DEVPERINT];
int sem_printer[DEVPERINT];
int sem_term_in[DEVPERINT];
int sem_term_out[DEVPERINT];

/**
 * @brief Gestisce gli inerrupt dei device
 * @param line La linea su cui viene tirato l'interrupt
 * @param semaphore Riferimento all'array dei semafori di quel device
 * */
static inline void generic_interrupt_handler(int line, int *semaphores);

inline void init_dev_sem(void)
{
  size_tt i;

  sem_it = 0;
  for (i = 0; i < DEVPERINT; ++i) {
    sem_disk[i] = sem_flash[i] = sem_net[i] = sem_printer[i] = sem_term_in[i] =
        sem_term_out[i] = 0;
  }
}


inline enum eh_act handle_interrupts(const int line)
{
  switch (line) {
  case IL_IPI: {
    break; /* safely ignore */
  }
  case IL_CPUTIMER: { /* PLT */
    /* Resetta il timer */
    setTIMER(TIMESLICE * (*(int *)(TIMESCALEADDR)));
      return RENQUEUE;
  }
  case IL_TIMER: {
    /* Pseudo-clock Tick */
  LDIT(PSECOND);
      pcb_t*p;

    while (sem_it != 1) {
      if((p=verhogen(&sem_it)) != NULL && p->p_semAdd !=NULL){
          LOGi("Verhogen has process still in semaphore ", p->p_pid);
        }
    }
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
  case IL_PRINTER: {
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

    size_tt status[2] = {reg->transm_status, reg->recv_status};
    size_tt *command[2] = {&reg->transm_command, &reg->recv_command};
    for (int i = 0; i < 2; ++i) {
        if((status[i]&TERMSTATMASK) == 5) {
          pcb_t *p = verhogen(&sem[i][index]);
          if (p != NULL) {
            p->p_s.reg_v0 = status[i];
          }
          /* Manda l'ack */
          *command[i] = 1;
          break;
        }
    }
    break;
  }
  default:
    break;
  }
  return CONTINUE;
}

inline void generic_interrupt_handler(int line, int *semaphores)
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
  /* Manda ack */
  reg->command = 1;
}
