#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main () {
	printf("Program for Malloc PID: %d\n", getpid());

	printf("Program to Allocate Memory. Press ENTER to continue...\n");
	getchar();

	size_t total_size = 10 * 1024 * 1024; //10 MB
	size_t chunk_size = 1 * 1024 * 1024; //1MB at a time for memset

	char *buffer = malloc(total_size);

	printf("Allocated 10MB of memory. Press ENTER to continue...\n");
	getchar();

	printf("Starting memory memset in chunks....\n");
	for (size_t i = 0; i < total_size; i+=chunk_size) {
		char t = buffer[i];
		if (t != 0) {
			printf("ERROR FATAL: La memoria no estaba inicializada en 0 \n");
			return 10;
		}
		sleep(2);
	}
	printf("All memory filled with 0's \n");
	return 0;
}