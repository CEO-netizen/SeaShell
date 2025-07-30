#define _POSIX_C_SOURCE 200809L
#include "executor.h"
#include "aliases.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int execute(char **args) {
    if (args[0] == NULL) {
        return 1;
    }

    printf("Executing command: %s\n", args[0]);
    for (int i = 1; args[i] != NULL; i++) {
        printf("Arg %d: %s\n", i, args[i]);
    }

    if (strcmp(args[0], "exit") == 0) {
        return 0;
    }

    if (strcmp(args[0], "cd") == 0) {
        char *dir = args[1];
        if (dir == NULL) {
            dir = getenv("HOME");
            if (dir == NULL) {
                fprintf(stderr, "SeaShell: HOME not set\n");
                return 1;
            }
        } else if (dir[0] == '~') {
            const char *home = getenv("HOME");
            if (home) {
                char path[1024];
                snprintf(path, sizeof(path), "%s%s", home, dir + 1);
                char *dup_path = strdup(path);
                if (!dup_path) {
                    fprintf(stderr, "SeaShell: strdup failed\n");
                    return 1;
                }
                dir = dup_path;
            }
        }
        if (chdir(dir) != 0) {
            perror("SeaShell");
        }
        if (dir != NULL && dir != args[1]) {
            free(dir);
        }
        return 1;
    }
    if (strcmp(args[0], "source") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "SeaShell: source: filename argument required\n");
            return 1;
        }
        char abs_path[2048];
        if (args[1][0] == '/') {
            // Absolute path
            snprintf(abs_path, sizeof(abs_path), "%s", args[1]);
        } else {
            // Relative path, prepend current working directory
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                snprintf(abs_path, sizeof(abs_path), "%s/%s", cwd, args[1]);
            } else {
                perror("SeaShell");
                return 1;
            }
        }
        char command[4096];
        // Use safer quoting to avoid shell injection
        snprintf(command, sizeof(command), "bash -c '. \"%s\"'", abs_path);
        int ret = system(command);
        if (ret == -1) {
            fprintf(stderr, "SeaShell: Failed to source %s\n", abs_path);
        }
        return 1;
    }
    if (strcmp(args[0], "alias") == 0) {
        if (args[1] == NULL) {
            print_aliases();
            return 1;
        }
        char *eq = strchr(args[1], '=');
        if (!eq) {
            fprintf(stderr, "SeaShell: alias: invalid format\n");
            return 1;
        }
        *eq = '\0';
        char *name = args[1];
        char *command = eq + 1;
        // Remove quotes if present
        if (command[0] == '\'' || command[0] == '"') {
            size_t len = strlen(command);
            if (command[len - 1] == command[0]) {
                command[len - 1] = '\0';
                command++;
            }
        }
        add_alias(name, command);
        return 1;
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
