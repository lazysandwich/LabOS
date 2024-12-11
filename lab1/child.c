#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s <имя файла>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *filename = argv[1];
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Ошибка при открытии файла");
        exit(EXIT_FAILURE);
    }
    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
        float sum = 0;
        char *token = strtok(buffer, " ");
        while (token != NULL) {
            sum += strtof(token, NULL);
            token = strtok(NULL, " ");
        }
        fprintf(file, "Сумма: %.2f\n", sum);
        fflush(file);
    }
    fclose(file);
    return 0;
}
