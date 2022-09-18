/**
 *
 * @file utils.h
 * @brief Implementazione delle funzioni che gestiscono la Active Semaphore List
 * (ASL)
 *
 */
#include "asl.h"
#include "listx.h"
#include "pandos_const.h"
#include "pcb.h"

static struct semd_t semd_table[MAXPROC];
static struct list_head *semd_free_h;
static struct list_head semd_free;
static struct list_head *semd_h;
static struct list_head semd;

/*Restituisce il semaforo corrispondente alla
chiave passata come input. Se non esiste tale
SEMD restituisce NULL.*/
static semd_t *get_semd(int *s_key)
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

int insert_blocked(int *semAdd, pcb_t *p)
{
  struct semd_t *s = get_semd(semAdd);
  if (s == NULL) { /*se il semaforo non è presente tra i SEMD*/
    if (list_empty(semd_free_h)) {
      /*termino restituendo TRUE se non è possibile allocare un nuovo semaforo
       * perchè la lista di quelli liberi è vuota*/
      return TRUE;
    } else {
      /*trasferisco un SEMD dai liberi agli utilizzati*/
      struct semd_t *tmp = container_of(semd_free_h->next, semd_t, s_link);
      tmp->s_key = semAdd;
      /*elimino il SEMD dalla lista dei liberi e lo inserisco nella lista dei
       * SEMD + inserisco il PCB*/
      list_del(semd_free_h->next);
      list_add(&(tmp->s_link), semd_h);
      INIT_LIST_HEAD(&(tmp->s_procq));
      insert_proc_q(&(tmp->s_procq), p);
    }
  } else { /*se il semaforo è già esistente inserisco il PCB*/
    insert_proc_q(&(s->s_procq), p);
  }

  p->p_semAdd = semAdd;

  /*restituisco FALSE come caso base*/
  return FALSE;
}

pcb_t *remove_blocked(int *semAdd)
{
  struct semd_t *s = get_semd(semAdd);

  if (s == NULL)
    return NULL;

  /*rimuovo il primo elemento bloccato su quel SEMD*/
  struct pcb_t *pcb = remove_proc_q(&(s->s_procq));
  pcb->p_semAdd = NULL;
  /*se la coda dei processi bloccati si svuota, trasferisco il semaforo nella
   * coda dei SEMD liberi*/
  if (list_empty(&(s->s_procq))) {
    list_del(&(s->s_link));
    list_add(&(s->s_link), semd_free_h);
  }

  return pcb;
}

pcb_t *out_blocked(pcb_t *p)
{
  if (p == NULL || p->p_semAdd == NULL) {
    return NULL;
  }

  struct semd_t *s = get_semd(p->p_semAdd);

  if (s == NULL) {
    return NULL;
  }

  p->p_semAdd = NULL;
  struct pcb_t *pcb =
      out_proc_q(&(s->s_procq), p); /*elimino il PCB p dalla lista*/
  if (pcb == NULL) {
    return NULL;
  }

  /*se la coda dei processi bloccati si svuota, trasferisco il semaforo nella
   * coda dei SEMD liberi*/
  if (list_empty(&(s->s_procq))) {
    list_del(&(s->s_link));
    list_add_tail(&(s->s_link), semd_free_h);
  }
  return pcb;
}

pcb_t *head_blocked(int *semAdd)
{
  struct semd_t *s = get_semd(semAdd);

  /*se SEMD non compare o se la sua lista dei processi è vuota return NULL*/
  if (s == NULL || list_empty(&(s->s_procq)))
    return NULL;

  /*restituisco il puntatore del PCB in testa alla coda*/
  return container_of((s->s_procq).next, pcb_t, p_list);
}

void init_asl(void)
{
  /*inizializzo liste per SEMD e SEMD liberi + associo i rispettivi puntatori*/
  INIT_LIST_HEAD(&semd);
  semd_h = &semd;
  INIT_LIST_HEAD(&semd_free);
  semd_free_h = &semd_free;

  /*inizializzo MAXPROC semafori liberi per contenere tutti gli elementi di
   * semdTable*/
  for (size_tt i = 0; i < MAXPROC; i++) {
    struct semd_t *tmp = &(semd_table[i]);
    INIT_LIST_HEAD(&(tmp->s_link));
    INIT_LIST_HEAD(&(tmp->s_procq));
    list_add_tail(&(tmp->s_link), semd_free_h);
  }
}
