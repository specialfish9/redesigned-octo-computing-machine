/*********************************P1TEST.C*******************************
 *
 *	Test program for the modules ASL and pcbQueues (phase 1).
 *
 *	Produces progress messages on terminal 0 in addition
 *		to the array ``okbuf[]''
 *		Error messages will also appear on terminal 0 in
 *		addition to the array ``errbuf[]''.
 *
 *		Aborts as soon as an error is detected.
 *
 *      Modified by Michael Goldweber on May 15, 2004
 */

#include "pandos_const.h"
#include "pandos_types.h"

#include "asl.h"
#include "pcb.h"
#include "term_utils.h"
#include <umps/libumps.h>

#define MAXPROC 20
#define MAXSEM MAXPROC

int sem[MAXSEM];
int onesem;
pcb_t *procp[MAXPROC], *p, *q, *firstproc, *lastproc, *midproc;

typedef unsigned int devreg;

int main(void)
{
  int i;

  print("Phase 1 test\n");
  init_pcbs();
  print("Initialized process control blocks   \n");

  /* Check allocProc */
  for (i = 0; i < MAXPROC; i++) {
    if ((procp[i] = alloc_pcb()) == NULL)
      print_err("alloc_pcb: unexpected NULL   ");
  }
  if (alloc_pcb() != NULL) {
    print_err("alloc_pcb: allocated more than MAXPROC entries   ");
  }
  print("alloc_pcb ok   \n");

  /* return the last 10 entries back to free list */

  for (i = 10; i < MAXPROC; i++)
    free_pcb(procp[i]);
  print("freed 10 entries   \n");

  /* create a 10-element process queue */
  LIST_HEAD(qa);
  if (!empty_proc_q(&qa))
    print_err("empty_proc_q: unexpected FALSE   ");
  print("Inserting...   \n");
  for (i = 0; i < 10; i++) {
    if ((q = alloc_pcb()) == NULL)
      print_err("alloc_pcb: unexpected NULL while insert   ");
    switch (i) {
    case 0:
      firstproc = q;
      break;
    case 5:
      midproc = q;
      break;
    case 9:
      lastproc = q;
      break;
    default:
      break;
    }
    insert_proc_q(&qa, q);
  }
  print("inserted 10 elements   \n");

  if (empty_proc_q(&qa))
    print_err("empty_proc_q: unexpected TRUE");

  /* Check outProc and headProc */

  if (head_proc_q(&qa) != firstproc)
    print_err("head_proc_q failed   ");
  q = out_proc_q(&qa, firstproc);
  if (q == NULL || q != firstproc)
    print_err("out_proc_q failed on first entry   ");
  free_pcb(q);

  q = out_proc_q(&qa, midproc);
  if (q == NULL || q != midproc)
    print_err("out_proc_q failed on middle entry   ");
  free_pcb(q);
  if (out_proc_q(&qa, procp[0]) != NULL)
    print_err("out_proc_q failed on nonexistent entry   ");
  print("out_proc_q ok   \n");

  /* Check if removeProc and insertProc remove in the correct order */

  print("Removing...   \n");
  for (i = 0; i < 8; i++) {
    if ((q = remove_proc_q(&qa)) == NULL)
      print_err("remove_proc_q: unexpected NULL   ");
    free_pcb(q);
  }

  if (q != lastproc)
    print_err("remove_proc_q: failed on last entry   ");
  if (remove_proc_q(&qa) != NULL)
    print_err("remove_proc_q: removes too many entries   ");

  if (!empty_proc_q(&qa))
    print_err("empty_proc_q: unexpected FALSE   ");

  print("insert_proc_q, remove_proc_q and empty_proc_q ok   \n");
  print("process queues module ok      \n");

  print("checking process trees...\n");

  if (!empty_child(procp[2]))
    print_err("empty_child: unexpected FALSE   ");
  /* make procp[1] through procp[9] children of procp[0] */
  print("Inserting...   \n");
  for (i = 1; i < 10; i++) {
    insert_child(procp[0], procp[i]);
  }
  print("Inserted 9 children   \n");

  if (empty_child(procp[0]))
    print_err("empty_child: unexpected TRUE   ");

  /* Check out_child */
  q = out_child(procp[1]);
  if (q == NULL || q != procp[1])
    print_err("out_child failed on first child   ");
  q = out_child(procp[4]);
  if (q == NULL || q != procp[4])
    print_err("out_child failed on middle child   ");
  if (out_child(procp[0]) != NULL)
    print_err("out_child failed on nonexistent child   ");
  print("out_child ok   \n");

  /* Check remove_child */
  print("Removing...   \n");
  for (i = 0; i < 7; i++) {
    if ((q = remove_child(procp[0])) == NULL)
      print_err("remove_child: unexpected NULL   ");
  }
  if (remove_child(procp[0]) != NULL)
    print_err("remove_child: removes too many children   ");

  if (!empty_child(procp[0]))
    print_err("empty_child: unexpected FALSE   ");

  print("insert_child, remove_child and empty_child ok   \n");
  print("process tree module ok      \n");

  for (i = 0; i < 10; i++)
    free_pcb(procp[i]);

  /* check ASL */
  init_asl();
  print("Initialized active semaphore list   \n");
  /* check remove_blocked and insert_blocked */
  print("insert_blocked test #1 started  \n");
  for (i = 10; i < MAXPROC; i++) {
    procp[i] = alloc_pcb();
    if (insert_blocked(&sem[i], procp[i]))
      print_err("insert_blocked(1): unexpected TRUE   ");
  }

  print("insert_blocked test #2 started  \n");
  for (i = 0; i < 10; i++) {
    procp[i] = alloc_pcb();
    if (insert_blocked(&sem[i], procp[i]))
      print_err("insert_blocked(2): unexpected TRUE   ");
  }

  /* check if semaphore descriptors are returned to free list */
  p = remove_blocked(&sem[11]);
  if (insert_blocked(&sem[11], p))
    print_err("remove_blocked: fails to return to free list   ");
  if (insert_blocked(&onesem, procp[9]) == FALSE)
    print_err("insert_blocked: inserted more than MAXPROC   ");
  print("remove_blocked test started   \n");
  for (i = 10; i < MAXPROC; i++) {
    q = remove_blocked(&sem[i]);
    if (q == NULL)
      print_err("remove_blocked: wouldn't remove   ");
    if (q != procp[i])
      print_err("remove_blocked: removed wrong element   ");
    if (insert_blocked(&sem[i - 10], q))
      print_err("insert_blocked(3): unexpected TRUE   ");
  }
  if (remove_blocked(&sem[11]) != NULL)
    print_err("remove_blocked: removed nonexistent blocked proc   ");
  print("insert_blocked and remove_blocked ok   \n");

  if (head_blocked(&sem[11]) != NULL)
    print_err("head_blocked: nonNULL for a nonexistent queue   ");
  if ((q = head_blocked(&sem[9])) == NULL)
    print_err("head_blocked(1): NULL for an existent queue   ");
  if (q != procp[9])
    print_err("head_blocked(1): wrong process returned   ");
  p = out_blocked(q);
  if (p != q)
    print_err("out_blocked(1): couldn't remove from valid queue   ");

  q = head_blocked(&sem[9]);
  if (q == NULL)
    print_err("head_blocked(2): NULL for an existent queue   ");
  if (q != procp[19])
    print_err("head_blocked(2): wrong process returned   ");
  p = out_blocked(q);
  if (p != q)
    print_err("out_blocked(2): couldn't remove from valid queue   ");
  p = out_blocked(q);
  if (p != NULL)
    print_err("out_blocked: removed same process twice.");
  if (head_blocked(&sem[9]) != NULL)
    print_err("out/head_blocked: unexpected nonempty queue   ");
  print("head_blocked and out_blocked ok   \n");
  print("ASL module ok   \n");
  print("So Long and Thanks for All the Fish\n");
  return 0;
}
