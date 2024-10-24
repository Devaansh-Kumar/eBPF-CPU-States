#define TASK_RUNNING			0x00000000
#define TASK_INTERRUPTIBLE		0x00000001
#define TASK_UNINTERRUPTIBLE		0x00000002

struct event {
    __u32 pid;
    __u32 state;
    char comm[TASK_COMM_LEN];
};

