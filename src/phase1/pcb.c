#include "pcb.h"
#include "listx.h"
#include "term_utils.h"

static pcb_t pcbFree_table[MAXPROC];
static struct list_head pcbFree_h;

/*Inizializza la lista pcbFree in modo da contenere tutti gli elementi della pcbFree_table.*/
void initPcbs(void)
{
  INIT_LIST_HEAD(&pcbFree_h);
  for (size_tt i = 0; i < MAXPROC; i++){
    list_add_tail(&pcbFree_table[i].p_list, &pcbFree_h);
  }
}

/*Inserisce il PCB puntato da p nella lista dei PCB liberi.*/
void freePcb(pcb_t *p)
{
  /*list_del(&p->p_list);*/
  list_add(&p->p_list, &pcbFree_h);
}

/*Restituisce NULL se la pcbFree_h è vuota.
Altrimenti rimuove un elemento dalla pcbFree, inizializza tutti i campi (NULL/0)
e restituisce l’elemento rimosso.*/
pcb_t *allocPcb(void)
{
  if (list_empty(&pcbFree_h))
    return NULL;
  pcb_t *pcb = container_of(pcbFree_h.next, pcb_t, p_list);
  list_del(pcbFree_h.next);

  /* Inizializzazione di tutti gli elementi */
  pcb->p_list.next = NULL;
  pcb->p_list.prev = NULL;
  pcb->p_parent = NULL;
  pcb->p_child.next = NULL;
  pcb->p_child.prev = NULL;
  pcb->p_sib.next = NULL;
  pcb->p_sib.prev = NULL;
  /* Impostazione di tutti gli elementi di processor state a 0*/
  pcb->p_s.entry_hi = 0;
  pcb->p_s.cause = 0;
  pcb->p_s.status = 0;
  pcb->p_s.pc_epc = 0;
  for(int i=0;i<STATE_GPR_LEN;i++)
    pcb->p_s.gpr[i] = 0;
  pcb->p_s.hi = 0;
  pcb->p_s.lo = 0;

  pcb->p_time = 0;
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

/*Restituisce il primo elemento nella lista. Se la lista è vuota il risultato è NULL.*/
pcb_t *headProcQ(struct list_head *head)
{
  if(emptyProcQ(head))
    return NULL;
  else
    return container_of(head->next, pcb_t, p_list);
}

/* Rimuove il primo elemento presente nella lista data. Se la lista è vuota il risultato è NULL.*/
pcb_t *removeProcQ(struct list_head *head)
{
  /* Controllo che la lista non sia vuota */
  if (emptyProcQ(head))
    return NULL;
  else {
    /* In caso contrario, ricavo il primo elemento della lista, lo elimino dalla lista e lo restituisco */
    pcb_t *pcb = headProcQ(head);
    list_del(head->next);
    return pcb;
  }
}

/* Elimina il pcb "p" dalla lista data e lo restituisce. Se p non è presente, il risultato è NULL. */
pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
  struct list_head *iter;

  /* Ciclo for che scorre l'intera lista */
  list_for_each(iter, head)
  {
    /* Se l'elemento della lista in esame punta al p_list del pcb che cerchiamo, esso viene eliminato e restituito. */
    if (iter == &p->p_list) {
      list_del(iter);
      return p;
    }
  }
  /*Se siamo arrivati alla fine del ciclo senza trovare p, il risultato è NULL */
  return NULL;
}
