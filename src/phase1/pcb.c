#include "pcb.h"
#include "listx.h"
#include "pandos_const.h"
#include "pandos_types.h"

/* PCB LIST */

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 *pcbFree_table.
 */
void initPcbs(void)
{
  pcbFree_h->p_list = LIST_HEAD_INIT(pcbFree_h->p_list);

  for (pcb_t *ptr = pcbFree_table; ptr != pcbFree_table + MAXPROC; ptr++) {
    list_add(&ptr->p_list, &pcbFree_h->p_list);
  }
}

/* PCB TREE */

/* Checks wheter p has children or not */
const int emptyChild(const pcb_t *p) { return list_empty(p->p_child.next); }

/* Inserts p as child of print */
void insertChild(pcb_t *prnt, pcb_t *p)
{
  if (!prnt->p_child.next) {
    INIT_LIST_HEAD(&prnt->p_child);
  }
  list_add(&p->p_sib, &prnt->p_child);
}

/* Removes first child of p */
pcb_t *removeChild(pcb_t *p)
{
  if (list_empty(&p->p_child)) {
    return NULL;
  }

  list_del(&p->p_child);
  return p;
}

/* Removes p from his parent's children */
pcb_t *outChild(pcb_t *p)
{
  pcb_t *parent = p->p_parent;

  if (!parent)
    return NULL;

  struct list_head *ptr;
  list_for_each(ptr, &p->p_sib)
  {
    pcb_t *curr = container_of(ptr, pcb_t, p_sib);
    if (curr == p) {
      list_del(ptr);
      return p;
    }
  }

  return p;
}
