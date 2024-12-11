#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256

int main() {
    int pipe1[2];
    pid_t pid;
    char filename[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    if (pipe(pipe1) == -1) {
        perror("Ошибка при создании pipe");
        exit(EXIT_FAILURE);
    }
    printf("Введите имя файла: ");
    if (fgets(filename, BUFFER_SIZE, stdin) == NULL) {
        perror("Ошибка ввода имени файла");
        exit(EXIT_FAILURE);
    }
    filename[strcspn(filename, "\n")] = '\0';
    pid = fork();
    if (pid == -1) {
        perror("Ошибка при создании процесса");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) { // Дочерний процесс
        close(pipe1[1]); // Закрываем запись в pipe1
        dup2(pipe1[0], STDIN_FILENO);
        close(pipe1[1]);
        execlp("./child", "./child", filename, NULL);
        perror("Ошибка при запуске дочернего процесса");
        exit(EXIT_FAILURE);
    }
    else { // Родительский процесс
        close(pipe1[0]); // Закрываем чтение из pipe1
        while (1) {
            printf("Введите числа через пробел (или 'exit' для выхода): ");
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
                perror("Ошибка ввода команды");
                break;
            }
            if (strncmp(buffer, "exit", 4) == 0) {
                break;
            }
            write(pipe1[1], buffer, strlen(buffer));
        }
        close(pipe1[1]);
        wait(NULL);
    }
    return 0;
}
