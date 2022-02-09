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

/* #include "asl.h" */
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
  initPcbs();
  print("Initialized process control blocks   \n");

  /* Check allocProc */
  for (i = 0; i < MAXPROC; i++) {
    if ((procp[i] = allocPcb()) == NULL)
      print_err("allocPcb: unexpected NULL   ");
  }
  if (allocPcb() != NULL) {
    print_err("allocPcb: allocated more than MAXPROC entries   ");
  }
  print("allocPcb ok   \n");

  /* return the last 10 entries back to free list */

  for (i = 10; i < MAXPROC; i++)
    freePcb(procp[i]);
  print("freed 10 entries   \n");

  /* create a 10-element process queue */
  LIST_HEAD(qa);
  if (!emptyProcQ(&qa))
    print_err("emptyProcQ: unexpected FALSE   ");
  print("Inserting...   \n");
  for (i = 0; i < 10; i++) {
    if ((q = allocPcb()) == NULL)
      print_err("allocPcb: unexpected NULL while insert   ");
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
    insertProcQ(&qa, q);
  }
  print("inserted 10 elements   \n");

  if (emptyProcQ(&qa))
    print_err("emptyProcQ: unexpected TRUE");

  /* Check outProc and headProc */

  if (headProcQ(&qa) != firstproc)
    print_err("headProcQ failed   ");
  q = outProcQ(&qa, firstproc);
  if (q == NULL || q != firstproc)
    print_err("outProcQ failed on first entry   ");
  freePcb(q);

  q = outProcQ(&qa, midproc);
  if (q == NULL || q != midproc)
    print_err("outProcQ failed on middle entry   ");
  freePcb(q);
  if (outProcQ(&qa, procp[0]) != NULL)
    print_err("outProcQ failed on nonexistent entry   ");
  print("outProcQ ok   \n");

  /* Check if removeProc and insertProc remove in the correct order */

  print("Removing...   \n");
  for (i = 0; i < 8; i++) {
    if ((q = removeProcQ(&qa)) == NULL)
      print_err("removeProcQ: unexpected NULL   ");
    freePcb(q);
  }

  if (q != lastproc)
    print_err("removeProcQ: failed on last entry   ");
  if (removeProcQ(&qa) != NULL)
    print_err("removeProcQ: removes too many entries   ");

  if (!emptyProcQ(&qa))
    print_err("emptyProcQ: unexpected FALSE   ");

  print("insertProcQ, removeProcQ and emptyProcQ ok   \n");
  print("process queues module ok      \n");

  print("checking process trees...\n");

  if (!emptyChild(procp[2]))
    print_err("emptyChild: unexpected FALSE   ");
   */
  /* make procp[1] through procp[9] children of procp[0] */
  print("Inserting...   \n");
  for (i = 1; i < 10; i++) {
    insertChild(procp[0], procp[i]);
  }
  print("Inserted 9 children   \n");

  if (emptyChild(procp[0]))
    print_err("emptyChild: unexpected TRUE   ");

  /* Check outChild */
  q = outChild(procp[1]);
  if (q == NULL || q != procp[1])
    print_err("outChild failed on first child   ");
  q = outChild(procp[4]);
  if (q == NULL || q != procp[4])
    print_err("outChild failed on middle child   ");
  if (outChild(procp[0]) != NULL)
    print_err("outChild failed on nonexistent child   ");
  print("outChild ok   \n");

  /* Check removeChild */
  print("Removing...   \n");
  for (i = 0; i < 7; i++) {
    if ((q = removeChild(procp[0])) == NULL)
      print_err("removeChild: unexpected NULL   ");
  }
  if (removeChild(procp[0]) != NULL)
    print_err("removeChild: removes too many children   ");

  if (!emptyChild(procp[0]))
    print_err("emptyChild: unexpected FALSE   ");

  print("insertChild, removeChild and emptyChild ok   \n");
  print("process tree module ok      \n");

  for (i = 0; i < 10; i++)
    freePcb(procp[i]);

  /* check ASL */
  initASL();
  print("Initialized active semaphore list   \n");
  /* check removeBlocked and insertBlocked */
  print("insertBlocked test #1 started  \n");
  for (i = 10; i < MAXPROC; i++) {
    procp[i] = allocPcb();
    if (insertBlocked(&sem[i], procp[i]))
      print_err("insertBlocked(1): unexpected TRUE   ");
  }
  print("insertBlocked test #2 started  \n");
  for (i = 0; i < 10; i++) {
    procp[i] = allocPcb();
    if (insertBlocked(&sem[i], procp[i]))
      print_err("insertBlocked(2): unexpected TRUE   ");
  }

  /* check if semaphore descriptors are returned to free list */
  p = removeBlocked(&sem[11]);
  if (insertBlocked(&sem[11], p))
    print_err("removeBlocked: fails to return to free list   ");

  if (insertBlocked(&onesem, procp[9]) == FALSE)
    print_err("insertBlocked: inserted more than MAXPROC   ");

  print("removeBlocked test started   \n");
  for (i = 10; i < MAXPROC; i++) {
    q = removeBlocked(&sem[i]);
    if (q == NULL)
      print_err("removeBlocked: wouldn't remove   ");
    if (q != procp[i])
      print_err("removeBlocked: removed wrong element   ");
    if (insertBlocked(&sem[i - 10], q))
      print_err("insertBlocked(3): unexpected TRUE   ");
  }
  if (removeBlocked(&sem[11]) != NULL)
    print_err("removeBlocked: removed nonexistent blocked proc   ");
  print("insertBlocked and removeBlocked ok   \n");

  if (headBlocked(&sem[11]) != NULL)
    print_err("headBlocked: nonNULL for a nonexistent queue   ");
  if ((q = headBlocked(&sem[9])) == NULL)
    print_err("headBlocked(1): NULL for an existent queue   ");
  if (q != procp[9])
    print_err("headBlocked(1): wrong process returned   ");
  p = outBlocked(q);
  if (p != q)
    print_err("outBlocked(1): couldn't remove from valid queue   ");
  q = headBlocked(&sem[9]);
  if (q == NULL)
    print_err("headBlocked(2): NULL for an existent queue   ");
  if (q != procp[19])
    print_err("headBlocked(2): wrong process returned   ");
  p = outBlocked(q);
  if (p != q)
    print_err("outBlocked(2): couldn't remove from valid queue   ");
  p = outBlocked(q);
  if (p != NULL)
    print_err("outBlocked: removed same process twice.");
  if (headBlocked(&sem[9]) != NULL)
    print_err("out/headBlocked: unexpected nonempty queue   ");
  print("headBlocked and outBlocked ok   \n");
  print("ASL module ok   \n");
  print("So Long and Thanks for All the Fish\n");
  return 0;
}
