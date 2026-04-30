#include "../include/prhs.h"
#include <stdarg.h>

LogEntry sim_log[MAX_LOG_ENTRIES];
int      log_count = 0;

void log_event(int tick, int task_id, const char* fmt, ...) {
    if (log_count >= MAX_LOG_ENTRIES) return;
    va_list args;
    va_start(args, fmt);
    sim_log[log_count].tick    = tick;
    sim_log[log_count].task_id = task_id;
    vsnprintf(sim_log[log_count].event, sizeof(sim_log[log_count].event), fmt, args);
    va_end(args);
    log_count++;
}

void print_log() {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║           SIMULATION EVENT LOG                       ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("  %-6s  %-8s  %s\n", "Tick", "Task ID", "Event");
    printf("  %-6s  %-8s  %s\n", "------", "-------", "----------------------------------------");
    for (int i = 0; i < log_count; i++) {
        printf("  %-6d  T%-7d  %s\n",
               sim_log[i].tick,
               sim_log[i].task_id,
               sim_log[i].event);
    }
    printf("\n");
}

void print_gantt(Scheduler* s) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║               GANTT CHART                            ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");

    // Find max finish time
    int max_t = 0;
    for (int i = 0; i < s->task_count; i++) {
        if (s->tasks[i].finish_time > max_t)
            max_t = s->tasks[i].finish_time;
    }

    // Print time header
    printf("  Task        |");
    for (int t = 0; t < max_t; t++) {
        printf("%2d", t);
    }
    printf("|\n  ------------|");
    for (int t = 0; t < max_t; t++) printf("--");
    printf("|\n");

    // Print each task's Gantt row
    for (int i = 0; i < s->task_count; i++) {
        Task* tk = &s->tasks[i];
        printf("  %-12s|", tk->name);
        for (int t = 0; t < max_t; t++) {
            if (t >= tk->start_time && t < tk->finish_time)
                printf("██");
            else
                printf("  ");
        }
        printf("|\n");
    }
    printf("\n");
}
