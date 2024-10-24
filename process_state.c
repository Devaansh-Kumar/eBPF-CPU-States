// process_state.c
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <signal.h>
#include "process_state.skel.h"

static volatile bool exiting = false;

#define TASK_COMM_LEN 16
#define TASK_RUNNING			0x00000000
#define TASK_INTERRUPTIBLE		0x00000001
#define TASK_UNINTERRUPTIBLE		0x00000002

struct event {
    int pid;
    int state;
    char comm[TASK_COMM_LEN];
};

void handle_event(void *ctx, int cpu, void *data, __u32 data_size) {
    struct event *e = data;

    const char *state_str = NULL;
    switch (e->state) {
        case TASK_RUNNING:
            state_str = "RUNNING/READY";
            break;
        case TASK_UNINTERRUPTIBLE: 
        case TASK_INTERRUPTIBLE:
            state_str = "WAITING";
            break;
        default:
            state_str = "UNKNOWN";
            break;
    }
    printf("Process: %s (PID: %d) is now in state: %s\n", e->comm, e->pid, state_str);
}

void handle_lost_event(void *ctx, int cpu, __u64 lost_cnt) {
    fprintf(stderr, "Lost %llu events on CPU %d\n", lost_cnt, cpu);
}

static void sig_handler(int signo) {
    exiting = true;
}

int main(int argc, char **argv) {
    struct process_state_bpf *skel;
    int err;

    // Set up signal handler for graceful exit
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    // Load and verify BPF application
    skel = process_state_bpf__open();
    if (!skel) {
        fprintf(stderr, "Failed to open BPF skeleton\n");
        return 1;
    }

    // Load & verify BPF programs
    err = process_state_bpf__load(skel);
    if (err) {
        fprintf(stderr, "Failed to load and verify BPF skeleton\n");
        return 1;
    }

    // Attach tracepoints
    err = process_state_bpf__attach(skel);
    if (err) {
        fprintf(stderr, "Failed to attach BPF skeleton\n");
        return 1;
    }

    // Set up the perf buffer
    struct perf_buffer *pb = perf_buffer__new(bpf_map__fd(skel->maps.events), 8, handle_event, handle_lost_event, NULL, NULL);
    if (!pb) {
        fprintf(stderr, "Failed to open perf buffer\n");
        return 1;
    }

    printf("Monitoring process state... Press Ctrl+C to exit.\n");

    // Poll the perf buffer
    while (!exiting) {
        err = perf_buffer__poll(pb, 100);
        if (err < 0 && err != -EINTR) {
            fprintf(stderr, "Error polling perf buffer: %d\n", err);
            break;
        }
    }

    // Clean up
    perf_buffer__free(pb);
    process_state_bpf__destroy(skel);

    return 0;
}
