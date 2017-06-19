#include <task.h>

#define NUM_TASKS 16

context_t all_tasks[NUM_TASKS];
int current_task = 0;

int
is_ready_task(int tasknum)
{
    context_t *ctx = &all_tasks[tasknum];
    return ctx->ready;
}

void
save_context(int tasknum, void *eip)
{
    context_t *ctx = &all_tasks[tasknum];
    asm_save_context(ctx, eip);
}

void
switch_executing_task(int target_task_num)
{
    context_t *stale_context = &all_tasks[current_task];
    context_t *target_context = &all_tasks[target_task_num];

    stale_context->ready = 1;
    asm_swap_context(stale_context, target_context);

    memset(stale_context, 0, sizeof(*stale_context));
}

void
yield(void)
{
    for (int i=current_task+1; i < current_task+NUM_TASKS; i++) {
        int tnum = i % NUM_TASKS;
        if (is_ready_task(tnum)) {
            switch_executing_task(tnum);
        }
    }
}

void
idle() {
    while (1) {
        // TODO put spinny on visible screen
        yield();
    }
}

void
clone_page_directory(context_t *new_ctx)
{
    physaddr_t new_cr3 = get_unused_page();
    uint32_t *old_pagedir = (void *) get_cr3();
    uint32_t *new_pagedir = (void *) new_cr3;  // assume identity mapping for now
    int first_user_page = IDENTITY_MAP_END >> 22;
    for (int i = first_user_page; i < 1024; ++i)
    {
        old_pagedir[i] = new_pagedir[i] = make_cow(old_pagedir[i]);
    }
    new_ctx->cr3 = new_cr3;
}

pagetable_entry_t
make_cow(pagetable_entry_t entry)
{
    // TODO: actually you know, make it cow
    return entry;
}

