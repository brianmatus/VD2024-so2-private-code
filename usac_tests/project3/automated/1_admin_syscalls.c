#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

#define SYS_MATUS_ADD_MEMORY_LIMIT 557
#define SYS_MATUS_GET_MEMORY_LIMITS 558
#define SYS_MATUS_UPDATE_MEMORY_LIMIT 559
#define SYS_MATUS_REMOVE_MEMORY_LIMIT 560

struct memory_limitation {
    pid_t pid;
    size_t memory_limit;
};

pid_t pid_B;
const char *pid_file = "./programA.pid";

int total_score = 0;

void func1_A();
void func2_A();
void func3_A();
void func4_A();

pid_t waiter_pids[3];

int current_function = 0;

void (*functions_A[])() = {func1_A, func2_A, func3_A, func4_A};
int total_functions = sizeof(functions_A) / sizeof(functions_A[0]);

void terminate_waiter_processes(pid_t *pids, int count) {
    for (int i = 0; i < count; i++) {
        if (kill(pids[i], SIGTERM) == -1) {
            perror("Failed to terminate waiter process");
        } else {
            printf("Terminated waiter_program with PID: %d\n", pids[i]);
        }
    }
}

void cleanup() {
    if (remove(pid_file) == 0) {
        printf("Clean up: Deleted %s\n", pid_file);
    } else {
        perror("Failed to delete PID file");
    }
    terminate_waiter_processes(waiter_pids, 3);
}

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        if (current_function >= total_functions) {
            return;
        }
        // printf("SYSCALLER: Executing function %d\n", current_function + 1);
        printf("-------------------------------------------------------\n");
        functions_A[current_function++]();
        // printf("SYSCALLER: Signaling ALLOCATOR\n");
        kill(pid_B, SIGUSR1);
    } else if (sig == SIGINT) {
        printf("Received SIGINT. Cleaning up before exit.\n");
        exit(0);
    }
}

void write_pid(const char *filename, pid_t pid) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Failed to write PID");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "%d", pid);
    fclose(file);
}

pid_t read_pid(const char *filename) {
    FILE *file = fopen(filename, "r");
    while (!file) {
        sleep(1);
        file = fopen(filename, "r");
    }
    pid_t pid;
    fscanf(file, "%d", &pid);
    fclose(file);
    return pid;
}

int main() {
    pid_t pid_A = getpid();
    write_pid(pid_file, pid_A);

    atexit(cleanup);

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    printf("SYSCALLER: Waiting for ALLOCATOR's PID...\n");
    pid_B = read_pid("./programB.pid");

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            // Child process
            execl("./waiter_program", "waiter_program", (char *)NULL);
            perror("Failed to exec ./waiter_program");
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            waiter_pids[i] = pid;
            printf("Spawned waiter_program with PID: %d\n", pid);
        }
    }


    printf("SYSCALLER: Starting tasks\n");
    printf("-------------------------------------------------------\n");
    functions_A[current_function++]();
    // printf("SYSCALLER: Signaling ALLOCATOR\n");
    kill(pid_B, SIGUSR1);

    while (current_function < total_functions) {
        pause();  // Wait for SIGUSR1 (or any I think)
    }
    printf("-------------------------------------------------------\n");
    printf("SYSCALLER: Finished all tasks\n");
    printf("Final score: %d\n",total_score);
    return 0;
}
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void func1_A() {
    if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, pid_B, 10 * 1024 * 1024) != 0) {
        printf("(1) Test  failed. add_memory_limit returned non-zero error %d\n", errno);
    }
    else {
        printf("(1) Test passed.\n");
        total_score++;
    }
    sleep(2);
}

void func2_A() {
    if (syscall(SYS_MATUS_UPDATE_MEMORY_LIMIT, pid_B, 30 * 1024 * 1024) != 0) {
        printf("(2) Test  failed. update_memory_limit returned non-zero error %d\n", errno);
    }
    else {
        printf("(2) Test passed.\n");
        total_score++;
    }
    sleep(2);
}

void func3_A() {
    struct memory_limitation *buffer = malloc(10 * sizeof(struct memory_limitation));
    int processes_returned;
    if (!buffer) {
        perror("(3) Allocation for memory_limitation buffer failed. This is not student error");
        exit(0);
    }

    /////////////////////Dummy1 100MB
    if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, waiter_pids[0], 100 * 1024 * 1024) != 0) {
        printf("(3) Test  failed. add_memory_limit1 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    /////////////////////Dummy2 1GB
    if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, waiter_pids[1], 1024L * 1024L * 1024L) != 0) {
        printf("(3) Test  failed. add_memory_limit2 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    /////////////////////Dummy2 1GB
    if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, waiter_pids[2], 421* 1024 * 1024) != 0) {
        printf("(3) Test  failed. add_memory_limit3 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    /////////////////////Get1
    if (syscall(SYS_MATUS_GET_MEMORY_LIMITS, buffer, 10, &processes_returned) != 0) {
        printf("(3) Test  failed. get_memory_limits1 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    bool test_print1_x, test_print1_d1, test_print1_d2, test_print1_d3;
    printf("++++++++++++++\n");
    for (int i = 0; i < processes_returned; ++i) {
        if (buffer[i].pid == pid_B) {
            test_print1_x = true;
            printf("pid_B exists.\n");
        } else if (buffer[i].pid == waiter_pids[0]) {
            test_print1_d1 = true;
            printf("dummy1 exists.\n");
        } else if (buffer[i].pid == waiter_pids[1]) {
            test_print1_d2 = true;
            printf("dummy2 exists.\n");
        } else if (buffer[i].pid == waiter_pids[2]) {
            test_print1_d3 = true;
            printf("dummy3 exists.\n");
        }
    }

    //Check if all 4 exist
    if (!(test_print1_x && test_print1_d1 && test_print1_d2 && test_print1_d3)) {
        printf("(3) Test  failed. Not all restrictions1 where present\n");
        goto func3_A_exit;
    }

    /////////////////////Remove dummy1
    if (syscall(SYS_MATUS_REMOVE_MEMORY_LIMIT, waiter_pids[0]) != 0) {
        printf("(3) Test  failed. remove_memory_limit1 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }


    /////////////////////Get2
    if (syscall(SYS_MATUS_GET_MEMORY_LIMITS, buffer, 10, &processes_returned) != 0) {
        printf("(3) Test  failed. get_memory_limits2 returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    test_print1_x  = false;
    test_print1_d2 = false;
    test_print1_d3 = false;
    printf("++++++++++++++\n");
    for (int i = 0; i < processes_returned; ++i) {
        if (buffer[i].pid == pid_B) {
            test_print1_x = true;
            printf("pid_B exists.\n");
        } else if (buffer[i].pid == waiter_pids[1]) {
            test_print1_d2 = true;
            printf("dummy2 exists.\n");
        } else if (buffer[i].pid == waiter_pids[2]) {
            test_print1_d3 = true;
            printf("dummy3 exists.\n");
        }
    }

    //Check if all 3 exist
    if (!(test_print1_x && test_print1_d2 && test_print1_d3)) {
        printf("(3) Test  failed. Not all restrictions2 where present\n");
        goto func3_A_exit;
    }

    /////////////////////Remove allocator
    if (syscall(SYS_MATUS_REMOVE_MEMORY_LIMIT, pid_B) != 0) {
        printf("(3) Test  failed. remove_memory_limitx returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    /////////////////////Get3
    if (syscall(SYS_MATUS_GET_MEMORY_LIMITS, buffer, 10, &processes_returned) != 0) {
        printf("(3) Test  failed. get_memory_limitsx returned non-zero error %d\n", errno);
        goto func3_A_exit;
    }

    test_print1_d2 = false;
    test_print1_d3 = false;
    printf("++++++++++++++\n");
    for (int i = 0; i < processes_returned; ++i) {
        if (buffer[i].pid == waiter_pids[1]) {
            test_print1_d2 = true;
            printf("dummy2 exists.\n");
        } else if (buffer[i].pid == waiter_pids[2]) {
            test_print1_d3 = true;
            printf("dummy3 exists.\n");
        }
    }

    //Check if all 2 exist
    if (!(test_print1_d2 && test_print1_d3)) {
        printf("(3) Test  failed. Not all restrictions3 where present\n");
        goto func3_A_exit;
    }

    printf("(3) Test passed.\n");
    total_score++;

func3_A_exit:
    sleep(2);
}

void func4_A() {
    if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, pid_B, 100 * 1024 * 1024) != 0) {
        printf("(4) Test  failed. add_memory_limit returned non-zero error %d\n", errno);
    }
    else {
        printf("(4) Test passed.\n");
        total_score++;
    }
    sleep(2);
}