# eBPF-CPU-States Tracker

This project uses eBPF to monitor and report the state of processes in the Linux kernel, such as when they are running, waiting, or transitioning between these states. The program attaches to various kernel tracepoints (such as `sched_switch`, `sched_process_wait`, and `sched_wakeup`) to track process state transitions.

## Features

- Monitor process state changes:
  - **Running** state: When a process is running or ready to run.
  - **Waiting** state: When a process is blocked (e.g., waiting for I/O).
  - **Waking up**: When a process moves from the waiting state back to the ready state.
- Uses eBPF and `libbpf` for efficient kernel-level event tracking.

## Prerequisites

- **libbpf**: The project depends on `libbpf` for loading and managing eBPF programs.
- **clang/llvm**: Used to compile the eBPF programs.
- **bpftool**: Required for generating BPF skeletons.

Install the necessary dependencies on Ubuntu:
```bash
sudo apt-get install clang llvm libbpf-dev bpftool gcc make
```

## Building and Running

```bash
sudo make all
sudo ./process_state
```

Example Output: 
```bash
Process: bash (PID: 1234) is now in state: RUNNING/READY
Process: some-process (PID: 5678) is now in state: WAITING
Process: some-process (PID: 5678) is now in state: RUNNING/READY
```

To clean up the compiled binaries and object files, run:
```bash
make clean
```

