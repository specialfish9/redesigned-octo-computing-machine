#ifndef PANDOS_TYPES_H_INCLUDED
#define PANDOS_TYPES_H_INCLUDED

/****************************************************************************
 *
 * This header file contains utility types definitions.
 *
 ****************************************************************************/

#include "listx.h"
#include "pandos_const.h"
#include <umps3/umps/types.h>

typedef signed int cpu_t;
typedef unsigned int memaddr;

/* Page Table Entry descriptor */
typedef struct pteEntry_t {
  unsigned int pte_entryHI;
  unsigned int pte_entryLO;
} pteEntry_t;

/* Support level context */
typedef struct context_t {
  unsigned int stackPtr;
  unsigned int status;
  unsigned int pc;
} context_t;

/* Support level descriptor */
typedef struct support_t {
  int sup_asid;                               /* process ID					*/
  state_t sup_exceptState[2];                 /* old state exceptions			*/
  context_t sup_exceptContext[2];             /* new contexts for passing up	*/
  pteEntry_t sup_privatePgTbl[USERPGTBLSIZE]; /* user page table */
  int sup_stackTLB[500];
  int sup_stackGen[500];
} support_t;

/* process table entry type */
typedef struct pcb_t {
  /* process queue  */
  struct list_head p_list;

  /* process tree fields */
  struct pcb_t *p_parent;   /* ptr to parent	*/
  struct list_head p_child; /* children list */
  struct list_head p_sib;   /* sibling list  */

  /* process status information */
  state_t p_s;     /* processor state */
  cpu_t p_time;    /* cpu time used by proc */
  cpu_t p_tm_updt; /* last p_time update */

  /* Pointer to the semaphore the process is currently blocked on */
  int *p_semAdd;

  /* Pointer to the support struct */
  support_t *p_supportStruct;

  /* Indicator of priority; 0 - low, 1 - high */
  int p_prio;

  /* process id */
  int p_pid;
} pcb_t, *pcb_PTR;

/* semaphore descriptor (SEMD) data structure */
typedef struct semd_t {
  /* Semaphore key */
  int *s_key;
  /* Queue of PCBs blocked on the semaphore */
  struct list_head s_procq;

  /* Semaphore list */
  struct list_head s_link;
} semd_t, *semd_PTR;

/* Page swap pool information structure type */
typedef struct swap_t {
  int sw_asid;        /* ASID number			*/
  int sw_pageNo;      /* page's virt page no.	*/
  pteEntry_t *sw_pte; /* page's PTE entry.	*/
} swap_t;

#endif
