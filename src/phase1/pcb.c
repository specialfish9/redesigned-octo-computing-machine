#include "pcb.h"
#include "listx.h"

static pcb_t pcbFree_table[MAXPROC];
static struct list_head* pcbFree_h;

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 *pcbFree_table.
 */
void initPcbs(void)
{
  INIT_LIST_HEAD(pcbFree_h);

  for (size_tt i = 0; i < MAXPROC; i++)
    list_add_tail(pcbFree_table[i].p_list, pcbFree_h.p_list);
}

void freePcb(pcb_t *p)
{
  /*list_del(&p->p_list);*/
  list_add(p->p_list, pcbFree_h.p_list);
}

pcb_t *allocPcb()
{
  if (list_empty(pcbFree_h.p_list))
    return NULL;
  pcb_t *pcb = container_of(pcbFree_h.p_list->next, pcb_t, p_list);
  list_del(pcbFree_h.p_list->next);
  pcb->p_parent = NULL;
  pcb->p_s = 0;
  pcb->p_time = 0;
  pcb->p_semAdd = NULL;
  return pcb;
}

/*void mkEmptyProcQ(struct list_head *head){

}

int emptyProcQ (struct list_head *head){

}
*/
