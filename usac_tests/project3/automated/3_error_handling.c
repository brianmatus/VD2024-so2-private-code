#include <sys/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define SYS_MATUS_ADD_MEMORY_LIMIT 557
#define SYS_MATUS_GET_MEMORY_LIMITS 558
#define SYS_MATUS_UPDATE_MEMORY_LIMIT 559
#define SYS_MATUS_REMOVE_MEMORY_LIMIT 560

void main(int argc, char* argv[]) {
	pid_t my_pid = getpid();
	int total_score = 0;

	if (getuid() != 0) {
		printf("-------------------------------------------------------\n");
		syscall(SYS_MATUS_ADD_MEMORY_LIMIT, my_pid, 10 * 1024 * 1024);
		if (errno != EPERM) {
	        printf("(1) Test  failed. add_memory_limit was executed without being sudoer. Returned code: %d\n", errno);
	    }
	    else {
	        printf("(1) Test passed.\n");
	        total_score++;
	    }
	    printf("-------------------------------------------------------\n");

	    char *new_argv[argc+3];
	    new_argv[0] = "sudo";
	    for (int i = 0; i < argc; ++i) {
	    	new_argv[i+1] = argv[i];
	    }

	    new_argv[argc+1] = "0";
	    if (total_score) {
	    	new_argv[argc+1] = "1";
	    }

	    new_argv[argc+2] = NULL;

	    execvp("sudo", new_argv);
	    perror("Failed to elevate privileges. Try again. This was not user error???? :( ya no sale");
	    exit(EXIT_FAILURE);
	}

	if(argv[argc-1][0] == 49) {
		total_score++;
	}
	// printf("Retrieved total score:%d\n", total_score);
	//total_score = 1;

	printf("-------------------------------------------------------\n");
	syscall(SYS_MATUS_ADD_MEMORY_LIMIT, -my_pid, 10 * 1024 * 1024);
	if (errno != EINVAL) {
        printf("(2) Test  failed. add_memory_limit was executed with negative values. Returned code: %d\n", errno);
    }
    else {
        printf("(2) Test passed.\n");
        total_score++;
    }


    printf("-------------------------------------------------------\n");
	syscall(SYS_MATUS_ADD_MEMORY_LIMIT, 424242L, 10 * 1024 * 1024);
	if (errno != ESRCH) {
        printf("(3) Test  failed. add_memory_limit was executed with invalid PID. Returned code: %d\n", errno);
    }
    else {
        printf("(3) Test passed.\n");
        total_score++;
    }

    printf("-------------------------------------------------------\n");
	syscall(SYS_MATUS_REMOVE_MEMORY_LIMIT, my_pid);
	if (errno != 102) {
        printf("(4) Test  failed. delete_memory_limit was executed with PID not in list. Returned code: %d\n", errno);
    }
    else {
        printf("(4) Test passed.\n");
        total_score++;
    }

    printf("-------------------------------------------------------\n");
	syscall(SYS_MATUS_UPDATE_MEMORY_LIMIT, my_pid, 10 * 1024 * 1024);
	if (errno != 102) {
        printf("(5) Test  failed. update_memory_limit was executed with PID not in list. Returned code: %d\n", errno);
    }
    else {
        printf("(5) Test passed.\n");
        total_score++;
    }

    printf("-------------------------------------------------------\n");
	syscall(SYS_MATUS_ADD_MEMORY_LIMIT, my_pid, 20 * 1024 * 1024);
	syscall(SYS_MATUS_ADD_MEMORY_LIMIT, my_pid, 20 * 1024 * 1024); //should fail
	if (errno != 101) {
        printf("(6) Test  failed. add_memory_limit was executed with PID already in list. Returned code: %d\n", errno);
    }
    else {
        printf("(6) Test passed.\n");
        total_score++;
    }

    printf("-------------------------------------------------------\n");
    printf("Final Score: %d\n", total_score);


}
