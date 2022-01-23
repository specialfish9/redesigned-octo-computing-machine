#include "pcb.h"
#include "listx.h"

static pcb_t pcbFree_table[MAXPROC];
static struct list_head *pcbFree_h;

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 *pcbFree_table.
 */
void initPcbs(void)
{

  INIT_LIST_HEAD(pcbFree_h);

  for (size_tt i = 0; i < MAXPROC; i++)
    list_add_tail(&pcbFree_table[i].p_list, pcbFree_h);
}

void freePcb(pcb_t *p)
{
  /*list_del(&p->p_list);*/
  list_add(&p->p_list, pcbFree_h);
}

pcb_t *allocPcb(void)
{
  if (list_empty(pcbFree_h))
    return NULL;
  pcb_t *pcb = container_of(pcbFree_h.next, pcb_t, p_list);
  list_del(pcbFree_h->next);
  pcb->p_parent = NULL;
  /*TODO*/
  pcb->p_semAdd = NULL;
  return pcb;
}

void mkEmptyProcQ(struct list_head *head) { INIT_LIST_HEAD(head); }

int emptyProcQ(struct list_head *head) { return list_empty(head); }

void insertProcQ(struct list_head *head, pcb_t *p)
{
  list_add_tail(&p->p_list, &head);
}

pcb_t *headProcQ(struct list_head *head)
{
  return container_of(head->next, pcb_t, p_list);
}

pcb_t *removeProcQ(struct list_head *head)
{
  if (list_empty(head))
    return NULL;
  else {
    pcb_t *pcb = container_of(head->next, pcb_t, p_list);
    list_del(head);
    return pcb;
  }
}

pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
  struct list_head *iter;

  /* Ciclo for che scorre l'intera lista */
  list_for_each(iter, &head)
  {
    /* Da ogni elemento della lista si risale al pcb corrispondente*/
    current_pcb = container_of(iter, pcb_t, p_list);
    /* Se il pcb dell'elemento in esame è quello che cerchiamo, viene eliminato
     * dalla lista*/
    if (current_pcb == p) {
      list_del(iter);
      return p;
    }
  }
  /*Se siamo arrivati alla fine del ciclo senza trovare p, il risultato è NULL
   */
  return NULL;
}