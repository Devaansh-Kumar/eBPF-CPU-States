// process_state.bpf.c
#include "vmlinux.h"
#include "process_state.h"
#include <bpf/bpf_helpers.h>

// Perf buffer for events
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY);
    __uint(key_size, sizeof(__u32));
    __uint(value_size, sizeof(__u32));
} events SEC(".maps");

// Process goes to waiting state
SEC("tracepoint/sched/sched_process_wait")
int handle_sched_process_wait(struct trace_event_raw_sched_process_wait *ctx) {
    struct event event = {};

    event.pid = ctx->pid;  // Get PID of the waiting process
    event.state = TASK_UNINTERRUPTIBLE;  // Indicates waiting state
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    // Emit the waiting event
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));
    
    return 0;
}

// Process goes to running/ready state (sched_switch)
SEC("tracepoint/sched/sched_switch")
int handle_sched_switch(struct trace_event_raw_sched_switch *ctx) {
    struct event event = {};

    // Capture outgoing process (likely going to waiting or ready queue)
    event.pid = ctx->prev_pid;
    event.state = TASK_RUNNING;  // Leaving running state, may go to ready or waiting   
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    // Emit event for the previous task
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    // Capture incoming process (coming to running state)
    event.pid = ctx->next_pid;
    event.state = TASK_RUNNING;  // Next task is now scheduled to run
    bpf_get_current_comm(&event.comm, sizeof(event.comm));

    // Emit event for the next task
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    return 0;
}

// Process is waking up from waiting state
SEC("tracepoint/sched/sched_wakeup")
int handle_sched_wakeup(struct trace_event_raw_sched_wakeup_template *ctx) {
    struct event event = {};    

    event.pid = ctx->pid;
    event.state = TASK_RUNNING;  // Process is woken up, ready to run
    bpf_probe_read_str(event.comm, sizeof(event.comm), ctx->comm);

    // Emit the waking-up event
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &event, sizeof(event));

    return 0;
}

char LICENSE[] SEC("license") = "GPL";
