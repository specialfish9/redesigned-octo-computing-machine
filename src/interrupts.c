#include "interrupts.h"
#include "asl.h"
#include "listx.h"
#include "utils.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

static int sem_it;
static int sem_disk[DEVPERINT];
static int sem_flash[DEVPERINT];
static int sem_net[DEVPERINT];
static int sem_printer[DEVPERINT];
static int sem_term_in[DEVPERINT];
static int sem_term_out[DEVPERINT];

inline int *get_dev_sem(int index){
  switch(index/DEVPERINT){
    case 0:
      return &sem_disk[index%DEVPERINT];
      break;
    case 1:
      return &sem_flash[index%DEVPERINT];
      break;
    case 2:
      return &sem_net[index%DEVPERINT];
      break;
    case 3:
      return &sem_printer[index%DEVPERINT];
      break;
    case 4:
      return &sem_term_in[index%DEVPERINT];
      break;
    case 5:
      return &sem_term_out[index%DEVPERINT];
      break;
    default:
      return &sem_it;
  }
}

inline void init_dev_sem(void)
{
  size_tt i;

  sem_it = 0;
  for (i = 0; i < DEVPERINT; ++i) 
    sem_disk[i] = sem_flash[i] = sem_net[i] = sem_printer[i] = sem_term_in[i] = sem_term_out[i] = 0;
}

inline void handle_interrupts(const int line)
{

  print1("handling interrupts on line: ");
  print1_int(line);
  print1("\n");
  switch (line) {
  case 0: {
    break; /* safely ignore */
  }
  case ITINT: {
    /* Pseudo-clock Tick */
    LDIT(100000);
    while(remove_blocked(&sem_it))
        ;
    int *dev_sem=get_dev_sem(TIMER_SEM_INDEX);
    *dev_sem=0;
    break;
  }
  case DISKINT: { /* TODO */
    break;
  }
  case FLASHINT: { /* TODO */
    break;
  }
  case NETWINT: { /* TODO */
    break;
  }
  case PRNTINT: { /* TODO */
    break;
  }
  case TERMINT: { /* TODO */
    break;
  }
  default:
    break;
  }
}
