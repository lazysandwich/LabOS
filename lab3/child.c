#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NAME "/shared_memory_example"
#define SEM_PARENT "/sem_parent"
#define SEM_CHILD "/sem_child"
#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *filename = argv[1];

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    char *shared_memory = mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_parent = sem_open(SEM_PARENT, 0);
    sem_t *sem_child = sem_open(SEM_CHILD, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    while (1) {
        sem_wait(sem_parent);
        if (strcmp(shared_memory, "exit") == 0) {
            break;
        }
        char *token = strtok(shared_memory, " ");
        float sum = 0;
        while (token != NULL) {
            sum += atof(token);
            token = strtok(NULL, " ");
        }
        FILE *file = fopen(filename, "a");
        if (!file) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "Сумма: %.2f\n", sum);
        fclose(file);
        sem_post(sem_child); // Уведомляем родителя, что данные обработаны
    }
    munmap(shared_memory, BUF_SIZE);
    sem_close(sem_parent);
    sem_close(sem_child);
    return 0;
}
