# Priority Resource Handling System (PRHS)

A simulation-based Operating Systems project that demonstrates priority inversion and its solutions: Priority Inheritance Protocol (PIP) and Priority Ceiling Protocol (PCP).

## Key Features
- **Priority Inversion Simulation**: Demonstrates how a low-priority task can block a high-priority task.
- **Priority Inheritance Protocol (PIP)**: Temporarily boosts a lock-holder's priority to resolve inversion.
- **Priority Ceiling Protocol (PCP)**: Preemptively raises priority upon resource acquisition to prevent inversion and deadlocks.
- **Preemptive Priority Scheduling**: Full simulation of tasks being preempted by higher-priority arrivals.

## Project Structure
- `src/`: Source code for the scheduler, resources, and logging.
- `include/`: Header files defining the core data structures (TCB, Resources).
- `docs/`: Detailed project report and analysis.
- `simulation.html`: Interactive web-based simulation for quick demonstrations.
- `Makefile`: Build system for GCC.

## Getting Started

### Prerequisites
- GCC (C11 support)
- POSIX pthread library

### Build and Run
```bash
make
./prhs
```

### Menu Options
1. Raw priority inversion (no fix)
2. Priority Inheritance (PIP) fix
3. Priority Ceiling (PCP) fix
4. Preemptive priority scheduling
5. Interactive mode (define custom tasks)

---
*Developed as an Operating Systems Project.*