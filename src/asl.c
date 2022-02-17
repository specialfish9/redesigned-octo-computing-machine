/*********************************asl.c****************************************
 *
 *  Implementazione delle funzioni che gestiscono la Active Semaphore List (ASL)
 *
 ******************************************************************************/
#include "asl.h"
#include "listx.h"
#include "pandos_const.h"
#include "pcb.h"

static struct semd_t semd_table[MAXPROC];
static struct list_head *semdFree_h;
static struct list_head semdFree;
static struct list_head *semd_h;
static struct list_head semd;

/*Restituisce il semaforo corrispondente alla
chiave passata come input. Se non esiste tale
SEMD restituisce NULL.*/
static semd_t *getSemd(int *s_key)
{
  struct list_head *iter;

  list_for_each(iter, semd_h)
  { /*percorro tutta la lista di SEMD finchè non trovo il semaforo
       corrispondente a s_key*/
    struct semd_t *tmp = container_of(iter, semd_t, s_link);
    if (tmp->s_key == s_key)
      return tmp;
  }
  /*se non trovo il semaforo restituisco NULL*/
  return NULL;
}

int insertBlocked(int *semAdd, pcb_t *p)
{
  struct semd_t *s = getSemd(semAdd);

  if (s == NULL) { /*se il semaforo non è presente tra i SEMD*/
    if (list_empty(semdFree_h)) {
      /*termino restituendo TRUE se non è possibile allocare un nuovo semaforo
       * perchè la lista di quelli liberi è vuota*/
      return TRUE;
    } else {
      /*trasferisco un SEMD dai liberi agli utilizzati*/
      struct semd_t *tmp = container_of(semdFree_h->next, semd_t, s_link);
      tmp->s_key = semAdd;
      /*elimino il SEMD dalla lista dei liberi e lo inserisco nella lista dei
       * SEMD + inserisco il PCB*/
      list_del(semdFree_h->next);
      list_add(&(tmp->s_link), semd_h);
      insertProcQ(&(tmp->s_procq), p);
    }
  } else { /*se il semaforo è già esistente inserisco il PCB*/
    insertProcQ(&(s->s_procq), p);
  }

  p->p_semAdd = semAdd;

  /*restituisco FALSE come caso base*/
  return FALSE;
}

pcb_t *removeBlocked(int *semAdd)
{
  struct semd_t *s = getSemd(semAdd);

  if (s == NULL)
    return NULL;

  struct pcb_t *pcb = removeProcQ(
      &(s->s_procq)); /*rimuovo il primo elemento bloccato su quel SEMD*/

  /*se la coda dei processi bloccati si svuota, trasferisco il semaforo nella
   * coda dei SEMD liberi*/
  if (list_empty(&(s->s_procq))) {
    list_del(&(s->s_link));
    list_add(&(s->s_link), semdFree_h);
  }
  return pcb;
}

pcb_t *outBlocked(pcb_t *p)
{
  struct semd_t *s = getSemd(p->p_semAdd);

  if (s == NULL)
    return NULL;

  struct pcb_t *pcb =
      outProcQ(&(s->s_procq), p); /*elimino il PCB p dalla lista*/

  /*se la coda dei processi bloccati si svuota, trasferisco il semaforo nella
   * coda dei SEMD liberi*/
  if (list_empty(&(s->s_procq))) {
    list_del(&(s->s_link));
    list_add(&(s->s_link), semdFree_h);
  }
  return pcb;
}

pcb_t *headBlocked(int *semAdd)
{
  struct semd_t *s = getSemd(semAdd);

  /*se SEMD non compare o se la sua lista dei processi è vuota return NULL*/
  if (s == NULL || list_empty(&(s->s_procq)))
    return NULL;

  /*restituisco il puntatore del PCB in testa alla coda*/
  return container_of((s->s_procq).next, pcb_t, p_list);
}

void initASL(void)
{
  /*inizializzo liste per SEMD e SEMD liberi + associo i rispettivi puntatori*/
  INIT_LIST_HEAD(&semd);
  semd_h = &semd;
  INIT_LIST_HEAD(&semdFree);
  semdFree_h = &semdFree;

  /*inizializzo MAXPROC semafori liberi per contenere tutti gli elementi di
   * semdTable*/
  for (size_tt i = 0; i < MAXPROC; i++) {
    struct semd_t *tmp = &(semd_table[i]);
    INIT_LIST_HEAD(&(tmp->s_link));
    INIT_LIST_HEAD(&(tmp->s_procq));
    list_add_tail(&(tmp->s_link), semdFree_h);
  }
}
