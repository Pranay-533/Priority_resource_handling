#include "../include/prhs.h"

// ═══════════════════════════════════════════════════════════════
//  DEMO 1: Classic Priority Inversion Scenario
//
//  Three tasks: High (T1), Medium (T2), Low (T3)
//  T3 holds a resource that T1 needs.
//  Without a fix: T2 can preempt T3 and delay T1 indefinitely.
//  With PIP or PCP: T3's priority is boosted so it finishes fast.
// ═══════════════════════════════════════════════════════════════
void demo_priority_inversion(bool apply_fix, SchedulingPolicy policy) {
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    if (!apply_fix) {
        printf("  DEMO: PRIORITY INVERSION (no protocol — raw problem)\n");
    } else {
        printf("  DEMO: PRIORITY INVERSION FIX (%s)\n",
               policy == POLICY_PRIORITY_INHERIT ? "Priority Inheritance" : "Priority Ceiling");
    }
    printf("════════════════════════════════════════════════════════════\n");

    SchedulingPolicy p = apply_fix ? policy : POLICY_NONE;
    Scheduler* s = scheduler_create(p, 50, true);

    // Add tasks: HIGH=1, MEDIUM=5, LOW=10
    scheduler_add_task(s, "T1_HIGH",   PRIORITY_HIGH, 3, 0);
    scheduler_add_task(s, "T2_MEDIUM", PRIORITY_MED,  4, 0);
    scheduler_add_task(s, "T3_LOW",    PRIORITY_LOW,  5, 0);

    // Add shared resource with ceiling = PRIORITY_HIGH (= 1)
    scheduler_add_resource(s, "SharedMem", PRIORITY_HIGH);

    printf("\n--- Phase 1: T3 (Low) acquires SharedMem ---\n");
    // T3 starts and takes the resource
    s->tasks[2].state = TASK_RUNNING;  // T3 is running
    resource_acquire(s, 2, 0);         // T3 grabs SharedMem

    printf("\n--- Phase 2: T1 (High) arrives and tries to acquire SharedMem ---\n");
    // T1 arrives and tries to grab same resource
    bool acquired = resource_acquire(s, 0, 0);
    if (!acquired) {
        printf("  >> Priority Inversion would occur here without a protocol!\n");
        printf("  >> T1 (HIGH priority) is blocked by T3 (LOW priority).\n");
        if (!apply_fix) {
            printf("  >> T2 (MEDIUM) can freely preempt T3, delaying T1 further!\n");
        }
    }

    printf("\n--- Phase 3: T3 releases SharedMem ---\n");
    resource_release(s, 2, 0);   // T3 releases the resource

    printf("\n--- Phase 4: Run full preemptive scheduler ---\n");
    // Reset states for full run
    for (int i = 0; i < s->task_count; i++) {
        s->tasks[i].state              = TASK_READY;
        s->tasks[i].effective_priority  = s->tasks[i].base_priority;
        s->tasks[i].waiting_for        = -1;
        s->tasks[i].held_count         = 0;
        s->tasks[i].start_time         = -1;
        s->tasks[i].finish_time        = -1;
        s->tasks[i].waiting_time       = 0;
        s->tasks[i].turnaround_time    = 0;
    }
    for (int i = 0; i < s->resource_count; i++)
        s->resources[i].held_by = -1;
    log_count = 0;

    scheduler_run(s);
    scheduler_print_stats(s);
    print_gantt(s);

    scheduler_destroy(s);
}

// ═══════════════════════════════════════════════════════════════
//  DEMO 2: Preemptive Priority Scheduling (no resources)
//  Shows how higher priority tasks preempt lower ones.
// ═══════════════════════════════════════════════════════════════
void demo_preemptive_scheduling() {
    printf("\n");
    printf("════════════════════════════════════════════════════════════\n");
    printf("  DEMO: PREEMPTIVE PRIORITY SCHEDULING\n");
    printf("════════════════════════════════════════════════════════════\n");

    Scheduler* s = scheduler_create(POLICY_NONE, 50, true);

    scheduler_add_task(s, "Task_A",  PRIORITY_LOW,  6, 0);
    scheduler_add_task(s, "Task_B",  PRIORITY_MED,  4, 0);
    scheduler_add_task(s, "Task_C",  PRIORITY_HIGH, 2, 0);

    // Simulate delayed arrivals
    s->tasks[1].state = TASK_READY;    // B arrives at tick 0
    s->tasks[2].state = TASK_READY;    // C arrives at tick 0

    scheduler_run(s);
    scheduler_print_stats(s);
    print_gantt(s);

    scheduler_destroy(s);
}
