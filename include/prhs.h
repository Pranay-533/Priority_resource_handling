/**
 * @file prhs.h
 * @brief Main header file for the Priority Resource Handling System (PRHS).
 * 
 * Defines the core structures for tasks, resources, and the scheduler,
 * along with function prototypes for simulation logic.
 */

#ifndef PRHS_H
#define PRHS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
#define MAX_TASKS       10      /**< Maximum number of tasks in the simulation */
#define MAX_RESOURCES   5       /**< Maximum number of shared resources */
#define MAX_NAME_LEN    32      /**< Maximum length for task/resource names */

// Priority levels (lower number = higher priority)
#define PRIORITY_HIGH   1       /**< Highest priority level */
#define PRIORITY_MED    5       /**< Medium priority level */
#define PRIORITY_LOW    10      /**< Lowest priority level */

/**
 * @brief Scheduling policies for resource contention.
 */
typedef enum {
    POLICY_NONE              = 0,   /**< No priority inversion fix */
    POLICY_PRIORITY_INHERIT  = 1,   /**< Priority Inheritance Protocol (PIP) */
    POLICY_PRIORITY_CEILING  = 2    /**< Priority Ceiling Protocol (PCP) */
} SchedulingPolicy;

/**
 * @brief Possible states for a task.
 */
typedef enum {
    TASK_READY     = 0,             /**< Task is in ready queue */
    TASK_RUNNING   = 1,             /**< Task is currently executing */
    TASK_BLOCKED   = 2,             /**< Task is waiting for a resource */
    TASK_DONE      = 3              /**< Task has completed execution */
} TaskState;

// ─────────────────────────────────────────────
//  Resource (Mutex-like)
// ─────────────────────────────────────────────

/**
 * @brief Structure representing a shared resource.
 */
typedef struct Resource {
    int         id;                 /**< Unique resource ID */
    char        name[MAX_NAME_LEN]; /**< Human-readable name */
    int         ceiling_priority;   /**< Highest priority of any task using this resource (for PCP) */
    int         held_by;            /**< ID of task holding this resource, -1 if free */
    pthread_mutex_t mutex;          /**< Internal mutex for thread safety (simulation-level) */
} Resource;

// ─────────────────────────────────────────────
//  Task Control Block
// ─────────────────────────────────────────────

/**
 * @brief Task Control Block (TCB) containing task metadata and metrics.
 */
typedef struct Task {
    int         id;                         /**< Unique task ID */
    char        name[MAX_NAME_LEN];         /**< Task name */
    int         base_priority;              /**< Original priority assigned at creation */
    int         effective_priority;         /**< Current priority after inheritance/ceiling boosts */
    int         period;                     /**< Periodic interval (ms) */
    int         burst_time;                 /**< Remaining CPU time required (ticks) */
    int         waiting_time;               /**< Cumulative ticks spent in READY state */
    int         turnaround_time;            /**< Total ticks from arrival to completion */
    int         start_time;                 /**< Tick when task first ran */
    int         finish_time;                /**< Tick when task finished */
    TaskState   state;                      /**< Current execution state */
    int         held_resources[MAX_RESOURCES]; /**< Array of resource IDs held by this task */
    int         held_count;                 /**< Number of resources currently held */
    int         waiting_for;                /**< ID of resource blocking this task, -1 if none */
} Task;

// ─────────────────────────────────────────────
//  Scheduler
// ─────────────────────────────────────────────

/**
 * @brief Main scheduler state for the simulation.
 */
typedef struct Scheduler {
    Task            tasks[MAX_TASKS];       /**< Task pool */
    int             task_count;             /**< Number of active tasks */
    Resource        resources[MAX_RESOURCES]; /**< Resource pool */
    int             resource_count;         /**< Number of available resources */
    SchedulingPolicy policy;                /**< Active protocol (None/PIP/PCP) */
    int             time_unit;              /**< Duration of a single simulation tick (ms) */
    bool            verbose;                /**< Enable detailed console logging */
} Scheduler;

// ─────────────────────────────────────────────
//  Log entry for simulation trace
// ─────────────────────────────────────────────

/**
 * @brief Data structure for tracking simulation events.
 */
typedef struct LogEntry {
    int     tick;                           /**< Simulation tick when event occurred */
    int     task_id;                        /**< ID of the task associated with the event */
    char    event[128];                     /**< Description of the event */
} LogEntry;

#define MAX_LOG_ENTRIES 1024
extern LogEntry sim_log[];
extern int      log_count;

// ─────────────────────────────────────────────
//  Function Prototypes
// ─────────────────────────────────────────────

// scheduler.c
/** @brief Creates and initializes a new scheduler instance. */
Scheduler* scheduler_create(SchedulingPolicy policy, int time_unit, bool verbose);
/** @brief Frees memory associated with the scheduler. */
void       scheduler_destroy(Scheduler* s);
/** @brief Adds a new task to the scheduler's pool. */
void       scheduler_add_task(Scheduler* s, const char* name, int priority, int burst_time, int period);
/** @brief Registers a new shared resource. */
void       scheduler_add_resource(Scheduler* s, const char* name, int ceiling_priority);
/** @brief Starts the simulation loop. */
void       scheduler_run(Scheduler* s);
/** @brief Prints performance statistics for all tasks. */
void       scheduler_print_stats(Scheduler* s);

// resource.c
/** @brief Attempts to acquire a resource for a task. Returns true if successful. */
bool   resource_acquire(Scheduler* s, int task_id, int res_id);
/** @brief Releases a resource held by a task and restores priority if needed. */
void   resource_release(Scheduler* s, int task_id, int res_id);
/** @brief Boosts holder's priority to match blocker's priority (PIP). */
void   apply_priority_inheritance(Scheduler* s, int blocked_task_id, int res_id);
/** @brief Boosts task's priority to resource ceiling immediately (PCP). */
void   apply_priority_ceiling(Scheduler* s, int task_id, int res_id);
/** @brief Recalculates and restores the correct priority after releasing a resource. */
void   restore_priority(Scheduler* s, int task_id);

// priority_queue.c
/** @brief Returns the ID of the task with the highest priority in READY state. */
int    get_highest_priority_ready_task(Scheduler* s);
/** @brief Checks if a higher priority task should preempt the current one. */
bool   preempt_if_needed(Scheduler* s, int current_task_id);

// logger.c
/** @brief Records an event in the simulation log. */
void   log_event(int tick, int task_id, const char* fmt, ...);
/** @brief Prints the event log to stdout. */
void   print_log();
/** @brief Renders a Gantt chart of the simulation. */
void   print_gantt(Scheduler* s);

// demo.c
/** @brief Runs a pre-configured scenario showing priority inversion or its fixes. */
void   demo_priority_inversion(bool apply_fix, SchedulingPolicy policy);
/** @brief Runs a scenario focusing on preemptive task scheduling. */
void   demo_preemptive_scheduling();

#endif // PRHS_H

