#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

void shell_loop();
char *read_line();
char **parse_line(char *line);
int execute(char **args);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    shell_loop();
    return 0;
}

void shell_loop() {
    char *line;
    char **args;
    int status = 1;

    do {
        printf("SeaShell> ");
        line = read_line();
        if (line == NULL) {
            printf("\n");
            break; // EOF (Ctrl+D)
        }
        args = parse_line(line);
        if (args[0] != NULL) {
            status = execute(args);
        }
        free(line);
        free(args);
    } while (status);
}

char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    if (getline(&line, &bufsize, stdin) == -1) {
        if (feof(stdin)) {
            free(line);
            return NULL;
        } else {
            perror("readline");
            exit(EXIT_FAILURE);
        }
    }
    return line;
}

char **parse_line(char *line) {
    int bufsize = MAX_ARGS, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "SeaShell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, DELIM);
    while (token != NULL) {
        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += MAX_ARGS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "SeaShell: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int execute(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("SeaShell");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("SeaShell");
    } else {
        // Parent process
        int status;
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}
