#include "task.h"
#include "virtualmem.h"
#include "asmhelpers.h"
#include "memlib.h"

#define NUM_TASKS 16

uint32_t swap_page[0xfff >> 2];

context_t all_tasks[NUM_TASKS];
int current_task = 0;

int
is_ready_task(int tasknum)
{
    context_t *ctx = &all_tasks[tasknum];
    return ctx->ready;
}

/* void */
/* save_context(int tasknum, void *eip) */
/* { */
/*     context_t *ctx = &all_tasks[tasknum]; */
/*     asm_save_context(ctx, eip); */
/* } */
/*  */
void
switch_executing_task(int target_task_num)
{
    context_t *stale_context = &all_tasks[current_task];
    context_t *target_context = &all_tasks[target_task_num];

    stale_context->ready = 1;
    asm_switch_to(stale_context, target_context);

    memset(stale_context, 0, sizeof(*stale_context));
}

// Wraps asm_fork
/* int */
/* fork() { */
/*  */
/* } */

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
   
    // Copy stack to swap page.
    // We're assuming the stack only occupies a single page.
    uint32_t *current_stack = (void *) BASE_OF_VIRTUAL_STACK;
    memcpy(swap_page, current_stack, STACK_SIZE);

    // Make cow from the first non-identity mapped page up to (but not including) the stack.
    for (int i = first_user_page; i < 1023; ++i)
    {
        old_pagedir[i] = new_pagedir[i] = make_cow(old_pagedir[i]);
    }

    physaddr_t new_phys_stack = get_unused_page();
    new_pagedir[1024] = make_present_and_rw(new_phys_stack);

    new_ctx->cr3 = new_cr3;
}

