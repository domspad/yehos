#ifndef YEHOS_TASK_H
#define YEHOS_TASK_H

typedef uint32_t reg_t;
typedef struct task_t {
    reg_t regs[16];
} task_t;

extern int current_task;

int is_ready_task(int t);

void switch_executing_task(int newlyExecutingTask);

void switch_visible_task(int newlyVisibleTask);

void save_context(int tasknum);  // for init
extern void *asm_save_context(void);
extern void asm_load_context(void *);

#endif
