# Priority Resource Handling System (PRHS)
## Operating Systems Project Report

---

## 1. Introduction

The **Priority Resource Handling System (PRHS)** is a simulation-based OS project that demonstrates:

- **Priority Inversion** — the classic problem where a low-priority task indirectly blocks a high-priority task
- **Priority Inheritance Protocol (PIP)** — a fix that temporarily boosts a lock-holder's priority
- **Priority Ceiling Protocol (PCP)** — a stronger fix that preemptively raises priority upon resource acquisition
- **Preemptive Priority Scheduling** — tasks can be preempted by higher-priority tasks at any time

---

## 2. Problem: Priority Inversion

### What is Priority Inversion?

Priority inversion occurs when a **high-priority task is blocked** by a **low-priority task** that holds a shared resource — and a **medium-priority task** further delays the low-priority task by preempting it.

### Classic Example

```
T1 (HIGH,  prio=1)  — needs SharedMem
T2 (MED,   prio=5)  — CPU-bound, no resource
T3 (LOW,   prio=10) — holds SharedMem
```

**Timeline without fix:**

```
Tick 0: T3 acquires SharedMem
Tick 1: T1 arrives → blocked (needs SharedMem) ← INVERSION
Tick 2: T2 arrives → preempts T3 (T2 has higher prio than T3)
Tick 2-5: T2 runs freely, T1 is STARVED even though T1 > T2 priority!
```

T1 should run before T2, but it effectively becomes the lowest-priority task.

---

## 3. Solutions Implemented

### 3.1 Priority Inheritance Protocol (PIP)

**Idea:** When a high-priority task blocks on a resource held by a low-priority task, the holder *inherits* the blocker's priority temporarily.

**Flow:**
1. T1 blocks on SharedMem (held by T3)
2. T3 inherits T1's priority (prio=1) temporarily
3. T3 now preempts T2 and finishes fast
4. T3 releases SharedMem → T3's priority restored to 10
5. T1 unblocks and runs

**Advantage:** Simple to implement  
**Disadvantage:** Doesn't prevent deadlocks; may cause chained inheritance

### 3.2 Priority Ceiling Protocol (PCP)

**Idea:** Each resource has a pre-defined *ceiling priority* = the highest priority of any task that could use the resource. When a task acquires the resource, it is immediately raised to that ceiling.

**Flow:**
1. T3 acquires SharedMem → T3 immediately boosted to ceiling (prio=1)
2. T2 cannot preempt T3 (T3 now has higher prio)
3. T3 finishes and releases SharedMem → priority restored
4. T1 acquires SharedMem, runs

**Advantage:** Prevents priority inversion AND deadlocks  
**Disadvantage:** Tasks may be unnecessarily boosted even without contention

---

## 4. Scheduling Algorithm

The scheduler implements **Preemptive Priority Scheduling**:

- A ready queue ordered by **effective priority** (lowest value = highest priority)
- At each tick, the highest-priority READY task runs
- If a higher-priority task becomes READY while another is RUNNING → preemption occurs
- Metrics tracked: **Waiting Time**, **Turnaround Time**, **Start/Finish Time**

### Formulas

```
Turnaround Time = Finish Time − Start Time
Waiting Time    = Turnaround Time − Burst Time
```

---

## 5. Project Structure

```
priority_resource_handling/
│
├── include/
│   └── prhs.h          ← All structs, enums, function prototypes
│
├── src/
│   ├── main.c          ← Entry point, interactive menu
│   ├── scheduler.c     ← Scheduler core (tick loop, statistics)
│   ├── resource.c      ← Resource acquire/release + PIP + PCP
│   ├── priority_queue.c← Ready-queue selection and preemption
│   ├── logger.c        ← Event log and Gantt chart printer
│   └── demo.c          ← Pre-built demo scenarios
│
├── docs/
│   └── report.md       ← This document
│
└── Makefile            ← Build system
```

---

## 6. How to Build and Run

### Requirements

- GCC (any version supporting C11)
- POSIX pthread library (standard on Linux/macOS)

### Build

```bash
make
```

### Run

```bash
./prhs
```

### Menu Options

| Option | Description |
|--------|-------------|
| 1 | Demonstrates raw priority inversion (no fix) |
| 2 | Demonstrates fix using Priority Inheritance (PIP) |
| 3 | Demonstrates fix using Priority Ceiling (PCP) |
| 4 | Demonstrates preemptive priority scheduling |
| 5 | Interactive mode — define your own tasks & resources |
| 0 | Exit |

---

## 7. Key Data Structures

### Task Control Block (TCB)

| Field | Purpose |
|-------|---------|
| `id` | Unique task identifier |
| `base_priority` | Original assigned priority |
| `effective_priority` | Current priority (may be boosted) |
| `burst_time` | Total CPU time needed |
| `state` | READY / RUNNING / BLOCKED / DONE |
| `held_resources[]` | Resources currently held |
| `waiting_for` | Resource this task is blocked on |

### Resource

| Field | Purpose |
|-------|---------|
| `id` | Unique resource identifier |
| `ceiling_priority` | Highest priority that uses this resource (for PCP) |
| `held_by` | Task ID currently holding this resource |

---

## 8. Sample Output

```
[Tick  0] RUNNING: 'T1_HIGH'  | Prio(eff)=1 | Remaining=3 tick(s)
[Tick  1] RUNNING: 'T1_HIGH'  | Prio(eff)=1 | Remaining=2 tick(s)
  [PIP] Task 'T3_LOW' (prio 10) inherits priority 1 from blocked Task 'T1_HIGH'
[Tick  2] RUNNING: 'T3_LOW'   | Prio(eff)=1 | Remaining=3 tick(s)   ← boosted!
...
```

---

---

## 10. Future Enhancements

Potential areas for further development of this system include:

1. **Multicore Support**: Extending the scheduler to handle multiple CPUs and global priority protocols (e.g., MrsP).
2. **Dynamic Priority Ceiling**: Implementing protocols that can adjust ceilings at runtime for more flexible resource management.
3. **Task Periodicity Logic**: Full implementation of Rate Monotonic Scheduling (RMS) for periodic task sets.
4. **GUI Visualization**: A web-based or desktop interface to visualize task preemption and resource locking in real-time.

---

## 11. Conclusion

This project simulates and solves the **priority inversion problem** — a real-world OS issue famously observed in NASA's Mars Pathfinder mission (1997). The implementation shows:

1. How priority inversion arises in a multitasking system
2. How **PIP** and **PCP** solve it at the cost of protocol overhead
3. How **preemptive priority scheduling** works in practice

Both PIP and PCP are used in real RTOS kernels (VxWorks, FreeRTOS, POSIX mutexes).
