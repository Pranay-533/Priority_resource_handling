#include "../include/prhs.h"

// ─────────────────────────────────────────────
//  Print main menu
// ─────────────────────────────────────────────
static void print_menu() {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║     PRIORITY RESOURCE HANDLING SYSTEM — MENU         ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║  1. Demo: Priority Inversion (No Fix)                ║\n");
    printf("║  2. Demo: Fix with Priority Inheritance (PIP)        ║\n");
    printf("║  3. Demo: Fix with Priority Ceiling (PCP)            ║\n");
    printf("║  4. Demo: Preemptive Priority Scheduling             ║\n");
    printf("║  5. Custom Simulation (Interactive)                  ║\n");
    printf("║  0. Exit                                             ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("  Choice: ");
}

// ─────────────────────────────────────────────
//  Interactive custom simulation
// ─────────────────────────────────────────────
static void run_custom_simulation() {
    int n_tasks, n_res;
    int policy_choice;

    printf("\n--- Custom Simulation Setup ---\n");
    printf("Choose scheduling policy:\n");
    printf("  0 = No Protocol (raw)\n");
    printf("  1 = Priority Inheritance (PIP)\n");
    printf("  2 = Priority Ceiling (PCP)\n");
    printf("  Policy: ");
    scanf("%d", &policy_choice);

    SchedulingPolicy policy = (SchedulingPolicy)policy_choice;
    Scheduler* s = scheduler_create(policy, 80, true);

    printf("Number of tasks (1-%d): ", MAX_TASKS);
    scanf("%d", &n_tasks);
    if (n_tasks < 1 || n_tasks > MAX_TASKS) n_tasks = 1;

    for (int i = 0; i < n_tasks; i++) {
        char name[MAX_NAME_LEN];
        int  prio, burst, period;
        printf("\nTask %d:\n", i + 1);
        printf("  Name      : "); scanf("%31s", name);
        printf("  Priority  (1=high, 10=low): "); scanf("%d", &prio);
        printf("  Burst time (ticks): ");          scanf("%d", &burst);
        printf("  Period     (0 if aperiodic): ");  scanf("%d", &period);
        scheduler_add_task(s, name, prio, burst, period);
    }

    printf("\nNumber of resources (0-%d): ", MAX_RESOURCES);
    scanf("%d", &n_res);
    if (n_res < 0 || n_res > MAX_RESOURCES) n_res = 0;

    for (int i = 0; i < n_res; i++) {
        char name[MAX_NAME_LEN];
        int  ceil_prio;
        printf("\nResource %d:\n", i + 1);
        printf("  Name            : ");   scanf("%31s", name);
        printf("  Ceiling priority: ");   scanf("%d", &ceil_prio);
        scheduler_add_resource(s, name, ceil_prio);
    }

    scheduler_run(s);
    scheduler_print_stats(s);
    print_gantt(s);
    scheduler_destroy(s);
}

// ─────────────────────────────────────────────
//  main
// ─────────────────────────────────────────────
int main(void) {
    int choice;

    printf("\n");
    printf("  ████████████████████████████████████████████████████\n");
    printf("  █                                                  █\n");
    printf("  █    PRIORITY RESOURCE HANDLING SYSTEM (PRHS)      █\n");
    printf("  █    OS Project — Priority Inversion & Scheduling  █\n");
    printf("  █                                                  █\n");
    printf("  ████████████████████████████████████████████████████\n");

    do {
        print_menu();
        if (scanf("%d", &choice) != 1) { choice = 0; break; }

        switch (choice) {
            case 1:
                demo_priority_inversion(false, POLICY_NONE);
                break;
            case 2:
                demo_priority_inversion(true, POLICY_PRIORITY_INHERIT);
                break;
            case 3:
                demo_priority_inversion(true, POLICY_PRIORITY_CEILING);
                break;
            case 4:
                demo_preemptive_scheduling();
                break;
            case 5:
                run_custom_simulation();
                break;
            case 0:
                printf("\n  Exiting PRHS. Goodbye!\n\n");
                break;
            default:
                printf("  Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
