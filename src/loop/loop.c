#include "loop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void shell_loop() {
    char *line;
    char **args;
    char **expanded_args;
    int status = 1;

    do {
        update_prompt();
        line = read_line();
        if (line == NULL) {
            printf("\n");
            break; // EOF (Ctrl+D)
        }
        args = parse_line(line);
        if (args == NULL) {
            free(line);
            continue;
        }
        if (args[0] != NULL) {
            expanded_args = expand_aliases(args);
            if (expanded_args == args) {
                // No alias expansion, do not free args before execute
                status = execute(expanded_args);
                free_args_array_only(args);
            } else {
                // Alias expanded, free args before execute
                free_args_array_only(args);
                status = execute(expanded_args);
                free_args(expanded_args);
            }
        } else {
            free_args_array_only(args);
        }
        free(line);
    } while (status);
}
