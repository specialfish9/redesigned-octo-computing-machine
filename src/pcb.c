#include "pcb.h"
#include "listx.h"
#include "term_utils.h"

static pcb_t pcbFree_table[MAXPROC];
static struct list_head pcbFree_h;


void initPcbs(void)
{/*Inizializzo la lista pcbFree_h come vuota, poi la riempio con gli elementi della pcbFree_table inserendoli in coda*/
  INIT_LIST_HEAD(&pcbFree_h);
  for (size_tt i = 0; i < MAXPROC; i++) {
    list_add_tail(&pcbFree_table[i].p_list, &pcbFree_h);
  }
}

void freePcb(pcb_t *p) { list_add(&p->p_list, &pcbFree_h); }

pcb_t *allocPcb(void)
{/*Controllo se pcbFree_h è vuota*/
  if (list_empty(&pcbFree_h))
    return NULL;
/*Se non è vuota copio i suoi elementi e la svuoto */
  pcb_t *pcb = container_of(pcbFree_h.next, pcb_t, p_list);
  list_del(pcbFree_h.next);

  /* Inizializzazione di tutti gli elementi */
  pcb->p_parent = NULL;

  INIT_LIST_HEAD(&(pcb->p_list));
  INIT_LIST_HEAD(&(pcb->p_child));
  INIT_LIST_HEAD(&(pcb->p_sib));

  /* Impostazione di tutti gli elementi di processor state a 0*/
  pcb->p_s.entry_hi = 0;
  pcb->p_s.cause = 0;
  pcb->p_s.status = 0;
  pcb->p_s.pc_epc = 0;
  for (int i = 0; i < STATE_GPR_LEN; i++)
    pcb->p_s.gpr[i] = 0;
  pcb->p_s.hi = 0;
  pcb->p_s.lo = 0;

  pcb->p_time = 0;
  pcb->p_semAdd = NULL;
  return pcb;
}

void mkEmptyProcQ(struct list_head *head) { INIT_LIST_HEAD(head); }

int emptyProcQ(struct list_head *head) { return list_empty(head); }

void insertProcQ(struct list_head *head, pcb_t *p){ list_add_tail(&p->p_list, head);}


pcb_t *headProcQ(struct list_head *head)
{
  /* Controllo che la lista non sia vuota */
  if (emptyProcQ(head))
    return NULL;
  else
    /* se non lo è restituisco il primo elemento */
    return container_of(head->next, pcb_t, p_list);
}


pcb_t *removeProcQ(struct list_head *head)
{
  /* Controllo che la lista non sia vuota */
  if (emptyProcQ(head))
    return NULL;
  else {
    /* Se non lo è, ricavo il primo elemento della lista, lo elimino dalla
     * lista e lo restituisco */
    pcb_t *pcb = headProcQ(head);
    list_del(head->next);
    return pcb;
  }
}


pcb_t *outProcQ(struct list_head *head, pcb_t *p)
{
  struct list_head *iter;

  /* Ciclo for che scorre l'intera lista */
  list_for_each(iter, head)
  {
    /* Se l'elemento della lista in esame punta al p_list del pcb che cerchiamo,
     * esso viene eliminato e restituito. */
    if (iter == &p->p_list) {
      list_del(iter);
      return p;
    }
  }
  /*Se siamo arrivati alla fine del ciclo senza trovare p, il risultato è NULL
   */
  return NULL;
}

/* PCB TREE */

/* Checks wheter p has children or not */
const int emptyChild(const pcb_t *p) { return list_empty(&(p->p_child)); }

/* Inserts p as child of print */
void insertChild(pcb_t *prnt, pcb_t *p)
{
  p->p_parent = prnt;
  list_add(&p->p_sib, &prnt->p_child);
}

/* Removes first child of p */
pcb_t *removeChild(pcb_t *p)
{
  struct list_head *tmp;
  pcb_t *first_child;

  if (list_empty(&p->p_child)) {
    return NULL;
  }

  tmp = list_next(&p->p_child);
  first_child = container_of(tmp, pcb_t, p_sib);

  list_del(tmp);
  first_child->p_parent = NULL;

  return first_child;
}

/* Removes p from his parent's children */
pcb_t *outChild(pcb_t *p)
{

  if (p->p_parent == NULL)
    return NULL;

  list_del(&p->p_sib);
  p->p_parent = NULL;

  return p;
}
