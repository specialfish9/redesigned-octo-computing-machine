/*********************************pcb.c****************************************
 *
 *  Implementazioni delle funzioni che gestiscono la coda e l'albero dei PCB.
 *
 ******************************************************************************/

#include "pcb.h"
#include "listx.h"
#include "pandos_types.h"
#include "term_utils.h"

static pcb_t pcb_free_table[MAXPROC];
static struct list_head pcb_free_h;

/* Inizializza la lista pcb_free in modo da contenere tutti gli elementi della
 * pcb_free_table.*/
void init_pcbs(void)
{
  /*Inizializzo la lista pcbFree_h come vuota, poi la riempio con gli elementi
   * della pcbFree_table inserendoli in coda*/
  INIT_LIST_HEAD(&pcb_free_h);
  for (size_tt i = 0; i < MAXPROC; i++) {
    list_add_tail(&pcb_free_table[i].p_list, &pcb_free_h);
  }
}

/* Inserisce il PCB puntato da p nella lista dei PCB liberi.*/
void free_pcb(pcb_t *p) { list_add(&p->p_list, &pcb_free_h); }

/* Restituisce NULL se pcb_free_h è vuota. Altrimenti rimuove un elemento da
 * pcb_free, inizializza tutti i campi (NULL/0) e restituisce l’elemento
 * rimosso.*/
pcb_t *alloc_pcb(void)
{
  /* Controllo se pcbFree_h è vuota */
  if (list_empty(&pcb_free_h))
    return NULL;

  /* Se non è vuota copio i suoi elementi e la svuoto */
  pcb_t *pcb = container_of(pcb_free_h.next, pcb_t, p_list);
  list_del(pcb_free_h.next);

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
  for (size_tt i = 0; i < STATE_GPR_LEN; i++)
    pcb->p_s.gpr[i] = 0;
  pcb->p_s.hi = 0;
  pcb->p_s.lo = 0;
  pcb->p_supportStruct = NULL;
  pcb->p_time = 0;
  pcb->p_semAdd = NULL;

  return pcb;
}

/* Crea una lista di PCB, inizializzandola come lista vuota */
void mk_empty_proc_q(struct list_head *head) { INIT_LIST_HEAD(head); }

/* Restituisce TRUE se la lista puntata da head è vuota, FALSE altrimenti.*/
int empty_proc_q(struct list_head *head) { return list_empty(head); }

/*Inserisce l’elemento puntato da p nella coda dei processi puntata da head.*/
void insert_proc_q(struct list_head *head, pcb_t *p)
{
  list_add_tail(&p->p_list, head);
}

/* Restituisce il primo elemento nella lista. Se la lista è vuota il risultato è
 * NULL.*/
pcb_t *head_proc_q(struct list_head *head)
{
  /* Controllo che la lista non sia vuota */
  if (empty_proc_q(head))
    return NULL;
  else
    /* se non lo è restituisco il primo elemento */
    return container_of(head->next, pcb_t, p_list);
}

/* Rimuove il primo elemento presente nella lista data. Se la lista è vuota il
 * risultato è NULL.*/
pcb_t *remove_proc_q(struct list_head *head)
{
  /* Controllo che la lista non sia vuota */
  if (empty_proc_q(head))
    return NULL;
  else {
    /* Se non lo è, ricavo il primo elemento della lista, lo elimino dalla
     * lista e lo restituisco */
    pcb_t *pcb = head_proc_q(head);
    list_del(&(pcb->p_list));
    INIT_LIST_HEAD(&(pcb->p_list));
    return pcb;
  }
}

/* Elimina il pcb "p" dalla lista data e lo restituisce. Se p non è presente, il
 * risultato è NULL. */
pcb_t *out_proc_q(struct list_head *head, pcb_t *p)
{
  struct list_head *iter;

  /* Ciclo for che scorre l'intera lista */
  list_for_each(iter, head)
  {
    /* Se l'elemento della lista in esame punta al p_list del pcb che cerchiamo,
     * esso viene eliminato e restituito. */
    if (iter == &(p->p_list)) {
      list_del(iter);
      INIT_LIST_HEAD(iter);
      return p;
    }
  }
  /*Se siamo arrivati alla fine del ciclo senza trovare p, il risultato è NULL
   */
  return NULL;
}

/* ALBERO DEI PCB */

/* Controlla se p ha figli */
const int empty_child(const pcb_t *p) { return list_empty(&(p->p_child)); }

/* Inseririsce p come figlio di prnt */
void insert_child(pcb_t *prnt, pcb_t *p)
{
  p->p_parent = prnt;
  list_add(&p->p_sib, &prnt->p_child);
}

/* Rimuove il primo figlio di p */
pcb_t *remove_child(pcb_t *p)
{
  struct list_head *tmp;
  pcb_t *first_child;

  /* Se non ha figli ritorna NULL */
  if (list_empty(&p->p_child)) {
    return NULL;
  }

  /* Ricava il primo elemento della lista dei figli di p */
  tmp = list_next(&p->p_child);
  first_child = container_of(tmp, pcb_t, p_sib);

  /* Cancellalo */
  list_del(tmp);
  /* Rimuovi il collegamento con il padre */
  first_child->p_parent = NULL;

  return first_child;
}

/* Rimuove p dai figli del padre */
pcb_t *out_child(pcb_t *p)
{
  /* Se p è root */
  if (p->p_parent == NULL)
    return NULL;

  /* Cancello i collegamenti di p con il resto del albero */
  list_del(&p->p_sib);
  p->p_parent = NULL;

  return p;
}
