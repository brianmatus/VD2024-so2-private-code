#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <time.h>

#define SYS_TAMALLOC 554
#define WAIT_CHUNK_SIZE 1024 * 1024


int main() {
    printf("Program for tamalloc PID: %d\n", getpid());

    printf("Program to Allocate Memory using tamalloc. Press ENTER to continue...\n");
    getchar();

    size_t total_size = 10 * 1024 * 1024; // 10 MB

    // Use the tamalloc syscall
    char *buffer = (char *)syscall(SYS_TAMALLOC, total_size);
    if ((long)buffer < 0) {
        perror("tamalloc failed");
        return 1;
    }
    printf("Allocated 10MB of memory using tamalloc at address: %p\n", buffer);

    printf("Press ENTER to start reading memory byte by byte...\n");
    getchar();

    srand(time(NULL));

    // Read memory byte by byte and verify it is zero
    for (size_t i = 0; i < total_size; i++) {
        if (buffer[i] != 0) { //triggers lazy allocation (with zeroing :D )
            printf("ERROR FATAL: Memory at byte %zu was not initialized to 0\n", i);
            return 10;
        }

        //Write a random A-Z char to trigger CoW
        //char random_letter = 'A' + (rand() % 26);
        char random_letter = 'A' + (i % 26);
        buffer[i] = random_letter;

        if (i % (WAIT_CHUNK_SIZE) == 0) { // Every 1 MB
            printf(".\n");
            sleep(1);
        }
    }

    printf("All memory verified to be zero-initialized. Press ENTER to exit.\n");
    getchar();
    return 0;
}
