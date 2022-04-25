/**
 *
 * @file kernel.h
 *
 */
#ifndef KERNEL_H
#define KERNEL_H

/**
 * @enum Rappresenta l'azione che l'exception handler deve compiere con il
 * processo in esecuzione una volta gestita un'eccezione.
 * */
enum eh_act {
  NOTHING = 0,  /* Non incodare, e quindi bloccare il processo corrente */
  RENQUEUE = 1, /* Rimettere in coda il processo corrente */
  CONTINUE = 2  /* Continuare con l'esecuzione del processo corrente */
};

#endif
