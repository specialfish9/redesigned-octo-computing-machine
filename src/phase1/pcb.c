#include "pcb.h"
#include "listx.h"
#include "pandos_types.h"

/*
 *Inizializza la lista pcbFree in modo da contenere tutti gli elementi della
 *pcbFree_table.
 */
void initPcbs(void)
{
  pcbFree_h->p_list = LIST_HEAD_INIT(pcbFree_h->p_list);

  for (int i = 0; i < MAXPROC; i++) {
    list_add(pcbFree_table[i], pcbFree_h->p_list);
  }

  for (pcb_t *ptr = pcbFree_table; ptr != NULL; ptr++) {
    list_add(ptr, pcbFree_h->p_list);
  }
}

const int emptyChild(const pcb_t *p) { return list_empty(&p->p_child); }

void insertChild(pcb_t *prnt, pcb_t *p)
{
  list_add(&p->p_list, &prnt->p_child);
}

pcb_t *removeChild(pcb_t *p)
{
  if (list_empty(&p->p_child)) {
    return NULL;
  }

  list_del(&p->p_child);
  return p;
}

pcb_t *outChild(pcb_t *p)
{
  pcb_t *father = p->p_parent;

  if (p == NULL)
    return NULL;
}
