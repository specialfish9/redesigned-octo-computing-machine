#include "term_utils.h"

#include "pandos_const.h"
#include "pandos_types.h"
#include <umps/libumps.h>

#define TRANSMITTED 5
#define ACK 1
#define PRINTCHR 2
#define CHAROFFSET 8
#define STATUSMASK 0xFF
#define TERM0ADDR 0x10000254

typedef unsigned int devreg;

static char okbuf[2048]; /* sequence of progress messages */
static char errbuf[128]; /* contains reason for failing */
static char *mp = okbuf;

/* This function returns the terminal transmitter status value given its address
 */
static devreg termstat(const memaddr *stataddr)
{
  return ((*stataddr) & STATUSMASK);
}

/* This function prints a string on specified terminal and returns TRUE if
 * print was successful, FALSE if not   */
static unsigned int termprint(const char *str, const unsigned int term)
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
void print(const char *strp)
{
  const char *tstrp = strp;
  while ((*mp++ = *strp++) != '\0')
    ;
  mp--;
  termprint(tstrp, 0);
}

/* This function placess the specified character string in errbuf and
 *	causes the string to be written out to terminal0.  After this is done
 *	the system shuts down with a panic message */
void print_err(const char *strp)
{
  char *ep = errbuf;
  const char *tstrp = strp;

  while ((*ep++ = *strp++) != '\0')
    ;

  termprint(tstrp, 0);

  PANIC();
}
