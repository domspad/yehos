#include "task.h"
#include "virtualmem.h"
#include "asmhelpers.h"
#include "memlib.h"

#define NUM_TASKS 16

uint32_t swap_page[0x1000 >> 2];

context_t all_tasks[NUM_TASKS];
int previous_task = 0;
int current_task = 0;

int
is_ready_task(int tasknum)
{
    context_t *ctx = &all_tasks[tasknum];
    return ctx->ready;
}

int
get_empty_task() {
    for (int i=current_task+1; i < current_task+NUM_TASKS; i++) {
        int tnum = i % NUM_TASKS;
        context_t *task = &all_tasks[tnum];
        if (task->cr3 == 0) {
          return tnum;
        }
    }
    // @TODO: something should happen if you're out of tasks
}

void
switch_executing_task(int target_task_num)
{
    previous_task = current_task;
    current_task = target_task_num;

    asm_switch_to(&all_tasks[previous_task], &all_tasks[current_task]);

    context_t *previous_task_struct = &all_tasks[previous_task];
    // We're now running target context,
    // so it shouldn't be in the task struct.
    memset(&all_tasks[current_task], 0, sizeof(all_tasks[0]));
    // We just saved to the stale context,
    // it's ready to be loaded at some point in the future.
    previous_task_struct->ready = 1;
}

/* Wraps asm_fork */
int
fork() {
    // the new task 
    previous_task = get_empty_task();

    int ret = asm_fork(&all_tasks[current_task], &all_tasks[previous_task]);

    context_t *previous_task_struct = &all_tasks[previous_task];
    // We're still running original context,
    // so it shouldn't be in the task struct.
    memset(&all_tasks[current_task], 0, sizeof(all_tasks[0]));
    previous_task_struct->ready = 1;
    return ret;
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
        draw_spinner();
        halt();
        yield();
    }
}

static char spinner_chars[4] = {'-','/','|','\\'};
static int spinner_idx = 0;

void
draw_spinner() {
    vga_setchar(79, 0, spinner_chars[spinner_idx], 0x03);
    spinner_idx = (spinner_idx + 1) % 4;
}

void
dup_context(context_t *stale_ctx, context_t *new_ctx) {
    memcpy(new_ctx, stale_ctx, sizeof(*stale_ctx));
    physaddr_t new_cr3 = clone_page_directory();
    new_ctx->cr3 = new_cr3;
}

