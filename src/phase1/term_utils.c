#include "term_utils.h"

char okbuf[2048]; /* sequence of progress messages */
char errbuf[128]; /* contains reason for failing */
char msgbuf[128]; /* nonrecoverable error message before shut down */
char *mp = okbuf;


typedef unsigned int devreg;

/* This function returns the terminal transmitter status value given its address
 */
static devreg termstat(memaddr *stataddr) { return ((*stataddr) & STATUSMASK); }

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
static unsigned int termprint(char *str, unsigned int term)
{
  memaddr *statusp;
  memaddr *commandp;
  devreg stat;
  devreg cmd;
  unsigned int error = FALSE;

  if (term < DEVPERINT) {
    /* terminal is correct */
    /* compute device register field addresses */
    statusp =
        (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANSTATUS * DEVREGLEN));
    commandp =
        (devreg *)(TERM0ADDR + (term * DEVREGSIZE) + (TRANCOMMAND * DEVREGLEN));

    /* test device status */
    stat = termstat(statusp);
    if (stat == READY || stat == TRANSMITTED) {
      /* device is available */

      /* print cycle */
      while (*str != EOS && !error) {
        cmd = (*str << CHAROFFSET) | PRINTCHR;
        *commandp = cmd;

        /* busy waiting */
        stat = termstat(statusp);
        while (stat == BUSY)
          stat = termstat(statusp);

        /* end of wait */
        if (stat != TRANSMITTED)
          error = TRUE;
        else
          /* move to next char */
          str++;
      }
    } else
      /* device is not available */
      error = TRUE;
  } else
    /* wrong terminal device number */
    error = TRUE;

  return (!error);
}

/* This function placess the specified character string in okbuf and
 *	causes the string to be written out to terminal0 */
void print(char *strp)
{
  char *tstrp = strp;
  while ((*mp++ = *strp++) != '\0')
    ;
  mp--;
  termprint(tstrp, 0);
}

/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void print_err(char *strp)
{
  char *ep = errbuf;
  char *tstrp = strp;

  while ((*ep++ = *strp++) != '\0')
    ;

  termprint(tstrp, 0);

  PANIC();
}

