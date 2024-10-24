# Compiler and tools
CLANG        = clang
LLC          = llc
BPFTOOL      = bpftool
CC           = gcc
CFLAGS       = -g -O2 -Wall -Werror
LIBS         = -lbpf -lelf -lz

# Target and source files
BPF_PROG     = process_state.bpf.o
USER_PROG    = process_state
BPF_SOURCE   = process_state.bpf.c
USER_SOURCE  = process_state.c
SKELETON     = process_state.skel.h

# Default target: build both BPF and user-space programs
all: $(USER_PROG)

# Compile eBPF program
$(BPF_PROG): $(BPF_SOURCE)
	$(CLANG) -target bpf -D__TARGET_ARCH_x86 -g -I/usr/include/$(shell uname -m)-linux-gnu -O2 -Wall -c $(BPF_SOURCE) -o $(BPF_PROG)

# Generate BPF skeleton header
$(SKELETON): $(BPF_PROG)
	$(BPFTOOL) gen skeleton $(BPF_PROG) > $(SKELETON)

# Compile user-space program, ensuring the skeleton is generated
$(USER_PROG): $(USER_SOURCE) $(SKELETON)
	$(CC) $(CFLAGS) $(USER_SOURCE) -o $(USER_PROG) $(LIBS)

# Clean build artifacts
clean:
	rm -f $(BPF_PROG) $(USER_PROG) $(SKELETON)

.PHONY: all clean
