#include "interrupts.h"
#include "asl.h"
#include "utils.h"
#include <umps3/umps/const.h>
#include <umps3/umps/libumps.h>
#include <umps3/umps/types.h>

inline void init_dev_sem(void)
{
  size_tt i;

  i = 0;
  while (i < DEV_SEM_LEN)
    dev_sem[i++] = 0;
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
    /* TODO VEHROGEN? */
    remove_blocked(&dev_sem[ITINT]);
    dev_sem[ITINT] = 0;

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
