#include <task.h>

#define NUM_TASKS 16

task_t all_tasks[NUM_TASKS];
int current_task = 0;

int
is_ready_task(int tasknum)
{
    task_t *t = &all_tasks[tasknum];
    return t->esp != 0;
}

void
save_context(int tasknum)
{
    task_t *t = &all_tasks[tasknum];
    t->esp = asm_save_context();
}

void
switch_executing_task(int tasknum)
{
    task_t *t = &all_tasks[current_task];

    t->esp = asm_save_context();
    if (t->esp) {
        // save_context returns twice: once when called and once when the context is re-loaded
        // in the first case, load the new context; in the second, it's already loaded.
        current_task = tasknum;
        asm_load_context(all_tasks[current_task].esp);
    }
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

