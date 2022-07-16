#ifndef VM_SUPPORT
#define VM_SUPPORT


extern int swp_pl_sem = 1;

extern void init_supp_structures(void);

extern void tlb_exc_handler(void);

#endif
