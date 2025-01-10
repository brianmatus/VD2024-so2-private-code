#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>

int main() {
    // Uses openat
    int fd1 = openat(AT_FDCWD, "file1.txt", O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd1 < 0) {
        perror("Error opening file1.txt with openat");
        exit(EXIT_FAILURE);
    }
    printf("Opened file1.txt with openat, file descriptor %d\n", fd1);

    // Uses openat
    int fd2 = openat(AT_FDCWD, "file2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd2 < 0) {
        perror("Error opening file2.txt with openat");
        close(fd1);
        exit(EXIT_FAILURE);
    }
    printf("Opened file2.txt with openat, file descriptor %d\n", fd2);

    // Direct call the open syscall
    int fd3 = syscall(SYS_open, "file3.txt", O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd3 < 0) {
        perror("Error opening file3.txt with open");
        close(fd1);
        close(fd2);
        exit(EXIT_FAILURE);
    }
    printf("Opened file3.txt with open (explicit syscall), file descriptor %d\n", fd3);

    // Direct call the open syscall
    int fd4 = syscall(SYS_open, "file4.txt", O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd4 < 0) {
        perror("Error opening file4.txt with open");
        close(fd1);
        close(fd2);
        close(fd3);
        exit(EXIT_FAILURE);
    }
    printf("Opened file4.txt with open (explicit syscall), file descriptor %d\n", fd4);

    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);

    printf("All files have been closed.\n");

    return 0;
}

