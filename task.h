#ifndef YEHOS_TASK_H
#define YEHOS_TASK_H

#include <stdint.h>

typedef uint32_t reg_t;
typedef struct context_t {
    reg_t ss, esp, cr3;
} context_t;

extern int current_task;

int is_ready_task(int t);

void switch_executing_task(int newlyExecutingTask);

void switch_visible_task(int newlyVisibleTask);

void save_context(int tasknum, void *func);
extern void asm_swap_context(context_t *fromctx, context_t *toctx);
extern void asm_save_context(context_t *fromctx, void *eip);


int
fork(void)
{
    context_t *new_ctx = &all_tasks[current_task]; // FIXME
    assert(new_ctx->cr3 == get_cr3());

    orig_cr3 = get_cr3();
    set_cr3(dup_addrspace(orig_cr3));

    ...

    set_cr3(orig_cr3);
}

physaddr_t
dup_addrspace(physaddr_t old_cr3)
{
    physaddr_t new_cr3 = get_unused_page();
    uint32_t *old_pagedir = (void *) old_cr3;
    uint32_t *new_pagedir = (void *) new_cr3;  // assume identity mapping for now
    for (int i=0; i < 1024; ++i)
    {
        new_pagedir[i] = make_cow(old_pagedir[i]);
    }
    return new_cr3;
}




#endif
