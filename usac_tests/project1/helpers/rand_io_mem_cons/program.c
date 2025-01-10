#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>


#define DISK_FILE "io_disk_test.txt"
#define MEM_SIZE 1024 * 1024 //1 MB
#define ITERATIONS 100


void random_disk_io() {
    FILE *file;
    char buffer[256];
    int random_offset, random_size;

    for (int i = 0; i < 256; ++i) {
        buffer[i] = 'A' + (rand() % 26);
    }

    file =fopen(DISK_FILE, "r+");
    if (!file) {
        file = fopen(DISK_FILE, "w+");
        if (!file) {
            perror("Error abriendo el archivo de disco");
        }
    }

    //Random seek and write
    random_offset = rand() % 1024;
    fseek(file, random_offset, SEEK_SET);
    fwrite(buffer, 1, sizeof(buffer), file);

    //Random seek and read
    random_offset = rand() % 1024;
    fseek(file, random_offset, SEEK_SET);
    fread(buffer, 1, sizeof(buffer), file);

    fclose(file); //TODO comentar para no forzar el flush del buffer?
}

void random_memory_io(char *memory) {
    int random_offset, random_size;

    random_offset = rand() % (MEM_SIZE - 256); //para evitar buffer overflow
    random_size = 256;

    memset(memory + random_offset, rand() % 256, random_size);

    char temp[256];
    memcpy(temp, memory + random_offset, random_size);
}


void random_console_output() {
    printf("Si salio el primer proyecto banda :D%d\n", rand());
}



int main() {
    char *memory;
    int choice; // disk I/O, mem I/O y console output

    srand(time(NULL));

    memory = (char*)malloc(MEM_SIZE);
    if (!memory) {
        perror("Error allocation 'memory'");
        return 1;
    }


    pid_t pid = getpid();
    printf("pid: %d\n", pid);



    for (int i = 0; i < ITERATIONS; ++i) {
        sleep(1);
        choice = rand() % 3;

        switch (choice) {
            case 0:
                random_disk_io();
                break;
            case 1:
                random_memory_io(memory);
                break;
            case 2:
                random_console_output();
                break;
        }
    }

    free(memory);
    remove(DISK_FILE);
    return 0;

}
