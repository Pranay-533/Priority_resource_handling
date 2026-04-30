#include "../include/prhs.h"

// ─────────────────────────────────────────────────────────────────
//  Priority Inheritance Protocol (PIP)
//  When a high-priority task is blocked on a resource held by a
//  lower-priority task, the holder temporarily inherits the higher
//  priority so it can finish and release the resource quickly.
// ─────────────────────────────────────────────────────────────────
void apply_priority_inheritance(Scheduler* s, int blocked_task_id, int res_id) {
    Resource* res = &s->resources[res_id];
    if (res->held_by < 0) return;

    Task* blocked = &s->tasks[blocked_task_id];
    Task* holder  = &s->tasks[res->held_by];

    if (holder->effective_priority > blocked->effective_priority) {
        printf("  [PIP] Task '%s' (prio %d) inherits priority %d from blocked Task '%s'\n",
               holder->name, holder->effective_priority,
               blocked->effective_priority, blocked->name);
        holder->effective_priority = blocked->effective_priority;
    }
}

// ─────────────────────────────────────────────────────────────────
//  Priority Ceiling Protocol (PCP)
//  When a task acquires a resource, its priority is raised to the
//  ceiling priority of that resource (the highest priority of any
//  task that could ever use the resource).
//  This prevents priority inversion AND deadlocks.
// ─────────────────────────────────────────────────────────────────
void apply_priority_ceiling(Scheduler* s, int task_id, int res_id) {
    Resource* res = &s->resources[res_id];
    Task*     tk  = &s->tasks[task_id];

    if (tk->effective_priority > res->ceiling_priority) {
        printf("  [PCP] Task '%s' priority raised from %d to ceiling %d (resource '%s')\n",
               tk->name, tk->effective_priority, res->ceiling_priority, res->name);
        tk->effective_priority = res->ceiling_priority;
    }
}

// ─────────────────────────────────────────────────────────────────
//  Restore a task's priority back to its base after releasing all
//  resources.
// ─────────────────────────────────────────────────────────────────
void restore_priority(Scheduler* s, int task_id) {
    Task* tk = &s->tasks[task_id];
    if (tk->effective_priority != tk->base_priority) {
        printf("  [RESTORE] Task '%s' priority restored: %d → %d\n",
               tk->name, tk->effective_priority, tk->base_priority);
        tk->effective_priority = tk->base_priority;
    }
}

// ─────────────────────────────────────────────────────────────────
//  Acquire a resource for a task
//  Returns true if acquired, false if blocked
// ─────────────────────────────────────────────────────────────────
bool resource_acquire(Scheduler* s, int task_id, int res_id) {
    Resource* res = &s->resources[res_id];
    Task*     tk  = &s->tasks[task_id];

    if (res->held_by == -1) {
        // Resource is free — acquire it
        res->held_by = task_id;
        tk->held_resources[tk->held_count++] = res_id;
        printf("  [ACQUIRE] Task '%s' acquired resource '%s'\n", tk->name, res->name);

        // Apply protocol
        if (s->policy == POLICY_PRIORITY_CEILING) {
            apply_priority_ceiling(s, task_id, res_id);
        }
        return true;
    } else {
        // Resource is held by another task — block
        printf("  [BLOCK] Task '%s' (prio %d) BLOCKED on resource '%s' held by Task '%s'\n",
               tk->name, tk->effective_priority, res->name,
               s->tasks[res->held_by].name);

        tk->state       = TASK_BLOCKED;
        tk->waiting_for = res_id;

        // Apply protocol
        if (s->policy == POLICY_PRIORITY_INHERIT) {
            apply_priority_inheritance(s, task_id, res_id);
        }
        return false;
    }
}

// ─────────────────────────────────────────────────────────────────
//  Release a resource held by a task
// ─────────────────────────────────────────────────────────────────
void resource_release(Scheduler* s, int task_id, int res_id) {
    Resource* res = &s->resources[res_id];
    Task*     tk  = &s->tasks[task_id];

    if (res->held_by != task_id) {
        printf("  [ERROR] Task '%s' does not hold resource '%s'\n", tk->name, res->name);
        return;
    }

    printf("  [RELEASE] Task '%s' released resource '%s'\n", tk->name, res->name);
    res->held_by = -1;

    // Remove from task's held list
    for (int i = 0; i < tk->held_count; i++) {
        if (tk->held_resources[i] == res_id) {
            tk->held_resources[i] = tk->held_resources[--tk->held_count];
            break;
        }
    }

    // Restore priority if no more resources held
    if (tk->held_count == 0) {
        restore_priority(s, task_id);
    }

    // Wake up any task blocked on this resource
    for (int i = 0; i < s->task_count; i++) {
        if (s->tasks[i].state == TASK_BLOCKED && s->tasks[i].waiting_for == res_id) {
            printf("  [UNBLOCK] Task '%s' unblocked, can now acquire '%s'\n",
                   s->tasks[i].name, res->name);
            s->tasks[i].state       = TASK_READY;
            s->tasks[i].waiting_for = -1;
        }
    }
}
