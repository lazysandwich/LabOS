#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <semaphore.h>

#define SHM_NAME "/shared_memory_example"
#define SEM_PARENT "/sem_parent"
#define SEM_CHILD "/sem_child"
#define BUF_SIZE 1024

int main() {
    char filename[BUF_SIZE];
    printf("Введите имя выходного файла: ");
    fgets(filename, BUF_SIZE, stdin);
    filename[strcspn(filename, "\n")] = '\0';
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, BUF_SIZE);
    char *shared_memory = mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    sem_t *sem_parent = sem_open(SEM_PARENT, O_CREAT, 0666, 0);
    sem_t *sem_child = sem_open(SEM_CHILD, O_CREAT, 0666, 0);
    if (sem_parent == SEM_FAILED || sem_child == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execl("./child", "./child", filename, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }
    while (1) {
        printf("Введите числа, разделенные пробелами (или 'exit' чтобы завершить): ");
        fgets(shared_memory, BUF_SIZE, stdin);
        shared_memory[strcspn(shared_memory, "\n")] = '\0';
        sem_post(sem_parent);
        if (strcmp(shared_memory, "exit") == 0) {
            break;
        }
        sem_wait(sem_child);
    }
    wait(NULL);
    munmap(shared_memory, BUF_SIZE);
    shm_unlink(SHM_NAME);
    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD);
    return 0;
}
