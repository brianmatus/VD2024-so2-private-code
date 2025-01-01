#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

#define SYS_MATUS_ADD_MEMORY_LIMIT 557
#define SYS_MATUS_GET_MEMORY_LIMITS 558
#define SYS_MATUS_UPDATE_MEMORY_LIMIT 559
#define SYS_MATUS_REMOVE_MEMORY_LIMIT 560

struct memory_limitation {
	pid_t pid;
	size_t memory_limit;
};


void add_memory_limit(pid_t pid, size_t memory_limit) {
	//TODO Chequeo de errores como en el enunciado

	if (syscall(SYS_MATUS_ADD_MEMORY_LIMIT, pid, memory_limit) < 0) {  //FIXME != 0
		perror("SYS_MATUS_ADD_MEMORY_LIMIT");
		return;
	}

	printf("Memory limit added for PID %d %zu bytes \n", pid, memory_limit);
}

void get_memory_limits(size_t max_entries) {
	struct memory_limitation *buffer = malloc(max_entries * sizeof(struct memory_limitation));
	int processes_returned;

	if (!buffer) {
		perror("Allocation for memory_limitation buffer failed");
		return;
	}

	if (syscall(SYS_MATUS_GET_MEMORY_LIMITS, buffer, max_entries, &processes_returned) < 0) {  //FIXME != 0
		perror("SYS_MATUS_GET_MEMORY_LIMITS");
		free(buffer);
		return;
	}

	printf("Restricted proccesses memory succesfully:\n");
	for (int i = 0; i < processes_returned; ++i) {
		printf("PID: %d, Memory Limit: %zu bytes\n", buffer[i].pid, buffer[i].memory_limit);
	}
	free(buffer);
}

void update_memory_limit(pid_t pid, size_t memory_limit) {
	if (syscall(SYS_MATUS_UPDATE_MEMORY_LIMIT, pid,  memory_limit) < 0) {  //FIXME != 0
		perror("SYS_MATUS_UPDATE_MEMORY_LIMIT");
		return;
	}
	printf("Memory limit for PID %d updated to %zu \n", pid, memory_limit);
}


void remove_memory_limit(pid_t pid) {
	if (syscall(SYS_MATUS_REMOVE_MEMORY_LIMIT, pid) < 0) {  //FIXME != 0
		perror("SYS_MATUS_REMOVE_MEMORY_LIMIT");
		return;
	}
	printf("Memory limit for PID %d removed\n", pid);
}



int main() {

	int choice;
	pid_t pid;
	size_t memory_limit;
	size_t max_entries;

	while (1) {
		printf("-----------------------------------------------------------------\n");
		printf("\nMemory Limitation for Project 3 SO2 VD2024\n");
		printf("1. Add Memory Limit\n");
		printf("2. Get Memory Limit\n");
		printf("3. Update Memory Limit\n");
		printf("4. Remove Memory Limit\n");
		printf("5. Exit\n");

		printf("Enter a number option to proceed\n");
		scanf("%d", &choice);

		printf("-----------------------------------------------------------------\n");

		switch(choice) {
			case 1:
				printf("Enter PID:");
				scanf("%d", &pid);
				printf("Enter Memory limit in KB:");
				scanf("%zu", &memory_limit);
				memory_limit *= 1024;
				add_memory_limit(pid, memory_limit);
				break;
			case 2:
				printf("Enter Max entries to receive:");
				scanf("%zu", &max_entries);
				get_memory_limits(max_entries);
				break;
			case 3:
				printf("Enter PID:");
				scanf("%d", &pid);
				printf("Enter Memory limit in KB:");
				scanf("%zu", &memory_limit);
				memory_limit *= 1024;
				update_memory_limit(pid, memory_limit);
				break;
			case 4:
				printf("Enter PID to remove limit:");
				scanf("%d", &pid);
				remove_memory_limit(pid);
				break;
			case 5:
				printf("Exiting...\n");
				return 0;
				break;
			default:
				printf("Invalid option. Try again!");
		}
	}

	return 0;
}
