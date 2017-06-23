#ifndef YEHOS_TASK_H
#define YEHOS_TASK_H

#include <asmhelpers.h>
#include <stdint.h>

typedef uint32_t reg_t;
typedef struct context_t {
    reg_t ss, esp, cr3;
    int ready;
} context_t;

extern int current_task;

int is_ready_task(int t);

void switch_executing_task(int newlyExecutingTask);

void switch_visible_task(int newlyVisibleTask);

void save_context(int tasknum, void *func);
extern void asm_swap_context(context_t *fromctx, context_t *toctx);
extern void asm_save_context(context_t *fromctx, void *eip);

void clone_page_directory(context_t *new_ctx);

void test_clone_page_directory(void);

#endif
