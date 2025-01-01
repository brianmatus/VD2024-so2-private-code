#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

pid_t pid_A;
const char *pid_file = "./programB.pid";

int total_score = 0;


char *buffer2, *buffer3, *buffer4;

void func1_B();
void func2_B();
void func3_B();
void func4_B();

int current_function = 0;

void (*functions_B[])() = {func1_B, func2_B, func3_B, func4_B};
int total_functions = sizeof(functions_B) / sizeof(functions_B[0]);

void cleanup() {
    if (remove(pid_file) == 0) {
        printf("Cleaned up: Deleted %s\n", pid_file);
    } else {
        perror("Failed to delete PID file");
    }
}

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        if (current_function >= total_functions) {
            return;   
        }

        // printf("ALLOCATOR: Executing function %d\n", current_function + 1);
        printf("-------------------------------------------------------\n");
        functions_B[current_function++]();
        // printf("ALLOCATOR: Signaling SYSCALLER\n");
        kill(pid_A, SIGUSR1);
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
    pid_t pid_B = getpid();
    write_pid("./programB.pid", pid_B);

    atexit(cleanup);

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    printf("ALLOCATOR: Waiting for SYSCALLER's PID...\n");
    pid_A = read_pid("./programA.pid");

    while (current_function < total_functions) {
        pause();  // Wait for SIGUSR1
    }
    printf("-------------------------------------------------------\n");
    printf("ALLOCATOR: Finished all tasks\n");
    printf("Final score: %d\n",total_score);
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
void func1_B() {
    printf("(1) ALLOCATOR: Task1-> Allocate 20 MB. Expected to fail\n");

    char *buffer = malloc(20 * 1024 * 1024);
    if (buffer) {
        printf("(1) Test  failed. Error. Call was expected to fail\n");
        free(buffer);
    }
    else {
        printf("(1) Test passed.\n");
        total_score++;
    }
    sleep(2);
}

void func2_B() {
    printf("(2) ALLOCATOR: Task1-> Allocate 20 MB. Expected to succeed\n");

    buffer2 = malloc(20 * 1024 * 1024);
    if (!buffer2) {
        printf("(2) Test  failed. Error. Call1 was expected to succeed\n");
        goto func2_B_exit;
    }

    printf("(2) ALLOCATOR: Task1-> Allocate 20 MB. Expected to fail\n");
    char *neg_buffer = malloc(20 * 1024 * 1024);
    if (neg_buffer) {
        printf("(2) Test  failed. Error. Call2 was expected to fail\n");
        free(neg_buffer);
        goto func2_B_exit;
    }

    printf("(2) Test passed.\n");
    total_score++;

func2_B_exit:
    sleep(2);
}

void func3_B() {
        printf("(3) ALLOCATOR: Task1-> Allocate 35 MB. Expected to succeed\n");

    buffer3 = malloc(35 * 1024 * 1024);
    if (!buffer2) {
        printf("(3) Test  failed. Error. Call1 was expected to succeed\n");
        goto func3_B_exit;
    }

    printf("(3) Test passed.\n");
    total_score++;

func3_B_exit:
    //Free the 55MB of memory
    if (buffer2) {
        free(buffer2);
        buffer2 = NULL;
    }
    if (buffer3) {
        free(buffer3);
        buffer3 = NULL;
    }
    sleep(2);
}


void func4_B() {
    printf("(4) ALLOCATOR: Task1-> Allocate 90 MB. Expected to succeed\n");

    buffer2 = malloc(90 * 1024 * 1024);
    if (!buffer2) {
        printf("(4) Test  failed. Error. Call1 was expected to succeed\n");
        goto func4_B_exit;
    }

    printf("(4) ALLOCATOR: Task1-> Allocate 20 MB. Expected to fail\n");
    char *neg_buffer = malloc(30 * 1024 * 1024);
    if (neg_buffer) {
        printf("(4) Test  failed. Error. Call2 was expected to fail\n");
        free(neg_buffer);
        goto func4_B_exit;
    }

    printf("(4) Test passed.\n");
    total_score++;

func4_B_exit:
    free(buffer2);
    sleep(2);
}