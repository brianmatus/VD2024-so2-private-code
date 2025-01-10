#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>

struct memory_snapshot {
    unsigned long total_pages;
    unsigned long free_pages;
    unsigned long active_pages;
    unsigned long cache_pages;
    unsigned long swap_pages;
};

// Define el número de la syscall (asegúrate de usar el correcto de syscall_64.tbl)
#define SYS_CAPTURE_MEMORY_SNAPSHOT 551

int main() {
    struct memory_snapshot snapshot;

    // Llama a la syscall
    long result = syscall(SYS_CAPTURE_MEMORY_SNAPSHOT, &snapshot);

    if (result < 0) {
        perror("Error al invocar la syscall");
        return EXIT_FAILURE;
    }

    // Imprime los resultados
    printf("Memory Snapshot:\n");
    printf("  Total Pages:   %lu\n", snapshot.total_pages);
    printf("  Free Pages:    %lu\n", snapshot.free_pages);
    printf("  Active Pages:  %lu\n", snapshot.active_pages);
    printf("  Cache Pages:   %lu\n", snapshot.cache_pages);
    printf("  Swap Pages:    %lu\n", snapshot.swap_pages);

    return EXIT_SUCCESS;
}

