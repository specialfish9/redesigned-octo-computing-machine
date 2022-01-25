#include "pcb.h"
#include "listx.h"

/*Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table.*/
void initPcbs(void)
{

  INIT_LIST_HEAD(pcbFree_h);

  for (size_tt i = 0; i < MAXPROC; i++)
    list_add_tail(&pcbFree_table[i].p_list, pcbFree_h);
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi.*/
void freePcb(pcb_t *p)
{
  /*list_del(&p->p_list);*/
  list_add(&p->p_list, pcbFree_h);
}

/*Restituisce NULL se la pcbFree_h è vuota.
Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i campi (NULL/0)
e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void)
{
  if (list_empty(pcbFree_h))
    return NULL;
  pcb_t *pcb = container_of(pcbFree_h->next, pcb_t, p_list);
  list_del(pcbFree_h->next);
  pcb->p_parent = NULL;
  /*TODO*/
  pcb->p_semAdd = NULL;
  return pcb;
}

/*Crea una lista di PCB, inizializzandola come lista vuota*/
void mkEmptyProcQ(struct list_head *head) { INIT_LIST_HEAD(head); }

/*Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.*/
int emptyProcQ(struct list_head *head) { return list_empty(head); }

/*Inserisce l’elemento puntato da p nella coda dei processi puntata da head.*/
void insertProcQ(struct list_head *head, pcb_t *p)
{
  list_add_tail(&p->p_list, head);
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
  list_for_each(iter, head)
  {
    /* Da ogni elemento della lista si risale al pcb corrispondente*/
   /* pcb_t *current_pcb = container_of(iter, pcb_t, p_list);*/
    /* Se il pcb dell'elemento in esame è quello che cerchiamo, viene eliminato
     * dalla lista*/
    if (iter == &p->p_list) {
      list_del(iter);
      return p;
    }
  }
  /*Se siamo arrivati alla fine del ciclo senza trovare p, il risultato è NULL
   */
  return NULL;
}