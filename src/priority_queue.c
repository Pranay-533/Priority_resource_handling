#include "../include/prhs.h"

// Returns task index with highest effective priority that is READY
// (lower effective_priority value = higher priority)
int get_highest_priority_ready_task(Scheduler* s) {
    int best = -1;
    int best_prio = 9999;
    for (int i = 0; i < s->task_count; i++) {
        if (s->tasks[i].state == TASK_READY) {
            if (s->tasks[i].effective_priority < best_prio) {
                best_prio = s->tasks[i].effective_priority;
                best = i;
            }
        }
    }
    return best;
}

// Check if a higher-priority ready task should preempt the current running task
// Returns true if preemption should occur
bool preempt_if_needed(Scheduler* s, int current_task_id) {
    if (current_task_id < 0) return false;
    Task* cur = &s->tasks[current_task_id];
    for (int i = 0; i < s->task_count; i++) {
        if (i == current_task_id) continue;
        if (s->tasks[i].state == TASK_READY) {
            if (s->tasks[i].effective_priority < cur->effective_priority) {
                if (s->verbose)
                    printf("  [PREEMPT] Task %s (prio %d) preempts Task %s (prio %d)\n",
                           s->tasks[i].name, s->tasks[i].effective_priority,
                           cur->name, cur->effective_priority);
                return true;
            }
        }
    }
    return false;
}
