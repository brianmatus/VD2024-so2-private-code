#include <stdio.h>
#include <time.h> // For struct timespec
#include <string.h> // For memset
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

#define SYSCALL_TRACK 552

struct syscall_usage {
    unsigned long count;
    struct timespec last_called;
};

#define SYS_CALL_ARRAY_SIZE 4

// Map syscall numbers to names
const char * syscall_names[SYS_CALL_ARRAY_SIZE] = {
    [0] = "read",
    [1] = "write",
    [2] = "open",
    [3] = "fork",
};

int main() {
    struct syscall_usage stats[SYS_CALL_ARRAY_SIZE];
    memset(stats, 0, sizeof(stats));

    // Perform some syscalls to test tracking
    char buffer[16];
    write(STDOUT_FILENO, "Test syscall write\n", 19); // Example write syscall
    read(STDIN_FILENO, buffer, sizeof(buffer));        // Example read syscall

    // Call the custom syscall to get stats
    if (syscall(SYSCALL_TRACK, stats) < 0) {
        perror("syscall");
        return 1;
    }

    // Display statistics for the tracked syscalls
    for (int i = 0; i < SYS_CALL_ARRAY_SIZE; i++) {
            printf("Syscall %s: Count=%lu, Last Called=%ld.%09ld\n",
                syscall_names[i],
                stats[i].count,
                stats[i].last_called.tv_sec,
                stats[i].last_called.tv_nsec);
    }

    return 0;
}

