#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Starting program that forks itself...");

    pid_t pid1 = fork();
    if (pid1 == 0) {
        //Children
        printf("Child lvl1 created. PID: %d, Parent PID: %d\n", getpid(), getppid());

    } else {
        printf("Parent after lvl1 fork. PID: %d\n", getpid());
    }
    
    sleep(4);

    pid_t pid2 = fork();
    if (pid2 == 0) {
        //Children
        printf("Child lvl2 created. PID: %d, Parent PID: %d\n", getpid(), getppid());

    } else {
        printf("Parent after lvl2 fork. PID: %d\n", getpid());
    }

    sleep(4);

    printf("Saliendo del proceso. PID: %d\n", getpid());
    return 0;

}
