#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <linux/unistd.h>
#include <unistd.h>
#include <errno.h>

#define SYSCALL_TRACK 553


struct io_stats {
    unsigned long bytes_read;
    unsigned long bytes_written;
    unsigned long bytes_read_disk;
    unsigned long bytes_written_disk;
    unsigned long io_wait_time;  // In ms
};


int main(int argc, char*argv[]) {

    //El primer argumento es el nombre del ejecutable. PID seria el segundo (index: 1)
    if (argc != 2) {
        printf("Numero de argumentos incorrecto. Debe especificar el PID: Ejemplo: ./test3 42\n");
        return 1;
    }

    pid_t pid = atoi(argv[1]);
    struct io_stats stats;

    int resultado = syscall(SYSCALL_TRACK, pid, &stats);
    if (resultado < 0) {
        perror("Error al llamar al syscall sys_matus_get_io_throttle");
        return errno;
    }

    printf("Estadisticas I/O para el PID %d:\n", pid);
    printf("Bytes leidos: %lu\n", stats.bytes_read);
    printf("Bytes escritos: %lu\n", stats.bytes_written);
    printf("Bytes leidos disco: %lu\n", stats.bytes_read_disk);
    printf("Bytes escritos disco: %lu\n", stats.bytes_written_disk);
    printf("Tiempo de espera I/O: %lu\n", stats.io_wait_time);

    return 0;
}

