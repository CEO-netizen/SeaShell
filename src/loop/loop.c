#include "loop.h"
#include <stdio.h>
#include <stdlib.h>

void shell_loop() {
    char *line;
    char **args;
    int status = 1;

    do {
        update_prompt();
        line = read_line();
        if (line == NULL) {
            printf("\n");
            break; // EOF (Ctrl+D)
        }
        args = parse_line(line);
        if (args[0] != NULL) {
            char **expanded_args = expand_aliases(args);
            free(args); // only free the array, not the strings
            status = execute(expanded_args);
            free_args(expanded_args);
        } else {
            free(args); // only free the array, not the strings
        }
        free(line);
    } while (status);
}
