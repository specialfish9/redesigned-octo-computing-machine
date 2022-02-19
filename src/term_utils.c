/******************************term_utils.c************************************
 *
 * Implementazione di term_utils.h.
 *
 ******************************************************************************/
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
#define MAX_BUF 2048
#define MAX_ERR_BUF 128

typedef unsigned int devreg;

static char okbuf[MAX_BUF];
static char errbuf[MAX_ERR_BUF];
static char *mp = okbuf;

/* Ritorna lo stato del terminale dato il suo indirizzo */
inline static devreg termstat(const memaddr *stataddr);
/* Stampa una stringa in un terminale specificato e ritorna TRUE se la stampa
 * ha avuto successo, FALSE altrimenti */
static unsigned int termprint(const char *str, const unsigned int term);

/* Funzioni su stringhe */

/* int to string */
static void _itoa(const int number, char *buffer);
/* Concatena due stringhe */
static void _strcat(const char *first, const char *second, char *buffer);
/* Inverte due caratteri */
inline static void _swap(char *x, char *y);

inline static devreg termstat(const memaddr *stataddr)
{
  return ((*stataddr) & STATUSMASK);
}

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

void print(const char *strp)
{
  const char *tstrp = strp;
  while ((*mp++ = *strp++) != '\0')
    ;
  mp--;
  termprint(tstrp, 0);
}

void print_err(const char *strp)
{
  char *ep = errbuf;
  const char *tstrp = strp;

  while ((*ep++ = *strp++) != '\0')
    ;

  termprint(tstrp, 0);

  PANIC();
}

void dbg_var(const char *name, const int var)
{
  char var_str[100], res[MAX_BUF];
  const char c[] = " : ";

  _itoa(var, var_str);
  _strcat(name, c, res);
  _strcat(res, var_str, res);
  print(res);
  print("\n");
}

/* Funzioni per la manipolazione di stringhe */

inline static void _swap(char *x, char *y)
{
  char t = *x;
  *x = *y;
  *y = t;
}

static void _itoa(const int number, char *buffer)
{
  size_tt n;
  char *buff_ptr, *i;

  if (!number) {
    *buffer = '0';
    *(buffer + 1) = '\0';
    return;
  }

  n = number < 0 ? -1 * number : number;
  buff_ptr = buffer;

  while (n) {
    *(buff_ptr++) = '0' + (n % 10);
    n /= 10;
  }

  if (number < 0)
    *(buff_ptr++) = '-';

  *buff_ptr = '\0';

  i = buffer;
  buff_ptr--;
  while (i < buff_ptr) {
    _swap(i++, buff_ptr--);
  }
}

static void _strcat(const char *first, const char *second, char *buffer)
{
  const char *ptr;
  char *buff_ptr;

  for (ptr = first, buff_ptr = buffer; *ptr; ptr++, buff_ptr++)
    *buff_ptr = *ptr;

  for (ptr = second; *ptr; ptr++, buff_ptr++)
    *buff_ptr = *ptr;

  *buff_ptr = '\0';
}
