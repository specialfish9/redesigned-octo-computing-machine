#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "listx.h"
#include "pandos_types.h"

extern void scheduler_next(pcb_t *current, const size_tt proc_count,
                           const size_tt sb_count, struct list_head *h_queue,
                           struct list_head *l_queue);

#endif
