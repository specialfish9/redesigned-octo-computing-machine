/**
 *
 * @file utils.c
 * @brief Funzioni di Utility.
 *
 */

#include "utils.h"
#include "klog.h"
#include "listx.h"

/* int to string */
static void _itoa(const int number, char *buffer);
/* Concatena due stringhe */
static void _strcat(const char *first, const char *second, char *buffer);
/* Inverte due caratteri */
inline static void _swap(char *x, char *y);

inline void memcpy(void *dest, void *src, size_tt n)
{
  size_tt i;
  char *csrc = (char *)src;
  char *cdest = (char *)dest;

  for (i = 0; i < n; i++)
    cdest[i] = csrc[i];
}

inline void log(char *tag, char *mex)
{
  kprint(tag);
  kprint(">");
  kprint(mex);
  kprint("\n");
}

inline void logi(char *tag, char *mex, int val)
{
  // char val_str[100];

  //_itoa(val, val_str);

  kprint(tag);
  kprint(">");
  kprint(mex);
  kprint_int(val);
  kprint("\n");
}

inline void logh(char *tag, char *mex, unsigned int value) 
{
  kprint(tag);
  kprint(">");
  kprint(mex);
  kprint_hex(value);
  kprint("\n");
}

void dbg_var(const char *name, const int var)
{
  char var_str[100], res[2048];
  const char c[] = " : ";

  _itoa(var, var_str);
  _strcat(name, c, res);
  _strcat(res, var_str, res);
  log("", res);
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
