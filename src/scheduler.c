#include "../include/prhs.h"

// ─────────────────────────────────────────────
//  Create and initialise scheduler
// ─────────────────────────────────────────────
Scheduler* scheduler_create(SchedulingPolicy policy, int time_unit, bool verbose) {
    Scheduler* s = (Scheduler*)calloc(1, sizeof(Scheduler));
    s->policy      = policy;
    s->time_unit   = time_unit;
    s->verbose     = verbose;
    s->task_count  = 0;
    s->resource_count = 0;

    // Init resource mutexes
    for (int i = 0; i < MAX_RESOURCES; i++) {
        s->resources[i].held_by = -1;
        pthread_mutex_init(&s->resources[i].mutex, NULL);
    }
    return s;
}

void scheduler_destroy(Scheduler* s) {
    for (int i = 0; i < s->resource_count; i++)
        pthread_mutex_destroy(&s->resources[i].mutex);
    free(s);
}

// ─────────────────────────────────────────────
//  Add a task to the scheduler
// ─────────────────────────────────────────────
void scheduler_add_task(Scheduler* s, const char* name, int priority, int burst_time, int period) {
    if (s->task_count >= MAX_TASKS) {
        printf("[WARN] Max tasks reached.\n");
        return;
    }
    int id = s->task_count++;
    Task* tk = &s->tasks[id];
    tk->id                = id;
    tk->base_priority     = priority;
    tk->effective_priority = priority;
    tk->burst_time        = burst_time;
    tk->period            = period;
    tk->state             = TASK_READY;
    tk->waiting_time      = 0;
    tk->turnaround_time   = 0;
    tk->start_time        = -1;
    tk->finish_time       = -1;
    tk->held_count        = 0;
    tk->waiting_for       = -1;
    strncpy(tk->name, name, MAX_NAME_LEN - 1);
}

// ─────────────────────────────────────────────
//  Add a resource to the scheduler
// ─────────────────────────────────────────────
void scheduler_add_resource(Scheduler* s, const char* name, int ceiling_priority) {
    if (s->resource_count >= MAX_RESOURCES) {
        printf("[WARN] Max resources reached.\n");
        return;
    }
    int id = s->resource_count++;
    Resource* r = &s->resources[id];
    r->id               = id;
    r->ceiling_priority = ceiling_priority;
    r->held_by          = -1;
    strncpy(r->name, name, MAX_NAME_LEN - 1);
}

// ─────────────────────────────────────────────
//  Print header banner
// ─────────────────────────────────────────────
static void print_banner(SchedulingPolicy policy) {
    const char* pname;
    switch (policy) {
        case POLICY_PRIORITY_INHERIT:  pname = "Priority Inheritance Protocol (PIP)"; break;
        case POLICY_PRIORITY_CEILING:  pname = "Priority Ceiling Protocol (PCP)";     break;
        default:                       pname = "No Protocol (Raw Priority Inversion)"; break;
    }
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║     PRIORITY RESOURCE HANDLING SYSTEM (PRHS)         ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  Policy: %-43s║\n", pname);
    printf("╚══════════════════════════════════════════════════════╝\n\n");
}

// ─────────────────────────────────────────────
//  Main simulation loop  (preemptive, tick-based)
// ─────────────────────────────────────────────
void scheduler_run(Scheduler* s) {
    print_banner(s->policy);

    int tick = 0;
    int remaining[MAX_TASKS];

    for (int i = 0; i < s->task_count; i++)
        remaining[i] = s->tasks[i].burst_time;

    int done_count = 0;
    int current_task = -1;

    while (done_count < s->task_count) {
        // Pick highest-priority ready task
        int next = get_highest_priority_ready_task(s);

        if (next == -1 && current_task == -1) {
            printf("[Tick %3d] All tasks blocked — idle tick\n", tick);
            tick++;
            usleep(s->time_unit * 1000);
            continue;
        }

        // Check if preemption is needed
        if (current_task != -1 && next != -1 && next != current_task) {
            Task* cur  = &s->tasks[current_task];
            Task* newt = &s->tasks[next];
            if (newt->effective_priority < cur->effective_priority) {
                // Preempt
                printf("[Tick %3d] PREEMPT: '%s' preempted by '%s'\n",
                       tick, cur->name, newt->name);
                cur->state   = TASK_READY;
                current_task = next;
            }
            // else keep running current_task
        } else if (current_task == -1) {
            current_task = next;
        }

        if (current_task == -1) {
            tick++;
            usleep(s->time_unit * 1000);
            continue;
        }

        Task* tk = &s->tasks[current_task];
        if (tk->start_time == -1) tk->start_time = tick;
        tk->state = TASK_RUNNING;

        printf("[Tick %3d] RUNNING: '%s' | Prio(eff)=%d | Remaining=%d tick(s)\n",
               tick, tk->name, tk->effective_priority, remaining[current_task]);

        // Accumulate waiting time for other READY tasks
        for (int i = 0; i < s->task_count; i++) {
            if (i != current_task && s->tasks[i].state == TASK_READY)
                s->tasks[i].waiting_time++;
        }

        remaining[current_task]--;

        if (remaining[current_task] == 0) {
            tk->state           = TASK_DONE;
            tk->finish_time     = tick + 1;
            tk->turnaround_time = tk->finish_time - tk->start_time;
            printf("[Tick %3d] DONE:    '%s' finished. Turnaround=%d, Wait=%d\n",
                   tick + 1, tk->name, tk->turnaround_time, tk->waiting_time);
            done_count++;
            current_task = -1;
        }

        tick++;
        usleep(s->time_unit * 1000);
    }

    printf("\n[Simulation complete at tick %d]\n", tick);
}

// ─────────────────────────────────────────────
//  Print final statistics table
// ─────────────────────────────────────────────
void scheduler_print_stats(Scheduler* s) {
    printf("\n╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║                     SCHEDULING STATISTICS                       ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    printf("║  %-12s  %6s  %6s  %6s  %12s  %12s  ║\n",
           "Task", "Prio", "Burst", "Period", "Waiting", "Turnaround");
    printf("║  %-12s  %6s  %6s  %6s  %12s  %12s  ║\n",
           "------------", "------", "------", "------", "------------", "------------");

    double avg_wait = 0, avg_turn = 0;
    for (int i = 0; i < s->task_count; i++) {
        Task* tk = &s->tasks[i];
        printf("║  %-12s  %6d  %6d  %6d  %12d  %12d  ║\n",
               tk->name, tk->base_priority, tk->burst_time, tk->period,
               tk->waiting_time, tk->turnaround_time);
        avg_wait += tk->waiting_time;
        avg_turn += tk->turnaround_time;
    }
    avg_wait /= s->task_count;
    avg_turn /= s->task_count;

    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    printf("║  Avg Waiting Time  : %-10.2f ticks                          ║\n", avg_wait);
    printf("║  Avg Turnaround    : %-10.2f ticks                          ║\n", avg_turn);
    printf("╚══════════════════════════════════════════════════════════════════╝\n\n");
}
