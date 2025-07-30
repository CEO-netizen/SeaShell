#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    if (!tokens) {
        fprintf(stderr, "SeaShell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    int length = strlen(line);
    int i = 0;
    while (i < length) {
        // Skip whitespace
        while (i < length && isspace((unsigned char)line[i])) i++;
        if (i >= length) break;

        char *token_start = &line[i];
        char *token;
        int token_len = 0;

        if (line[i] == '\'' || line[i] == '"') {
            char quote = line[i];
            i++; // skip opening quote
            token_start = &line[i];
            while (i < length && line[i] != quote) {
                if (line[i] == '\\' && i + 1 < length && line[i + 1] == quote) {
                    // Skip escape character
                    i += 2;
                    token_len++;
                } else {
                    i++;
                    token_len++;
                }
            }
            if (i >= length) {
                fprintf(stderr, "SeaShell: unmatched quote\n");
                free(tokens);
                return NULL;
            }
            // Null-terminate token
            line[i] = '\0';
            i++; // skip closing quote
            token = token_start;
        } else {
            token_start = &line[i];
            while (i < length && !isspace((unsigned char)line[i])) {
                i++;
                token_len++;
            }
            // Null-terminate token
            line[i] = '\0';
            i++;
            token = token_start;
        }

        tokens[position++] = token;

        if (position >= bufsize) {
            bufsize += MAX_ARGS;
            char **new_tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!new_tokens) {
                fprintf(stderr, "SeaShell: allocation error\n");
                free(tokens);
                exit(EXIT_FAILURE);
            }
            tokens = new_tokens;
        }
    }
    tokens[position] = NULL;
    return tokens;
}
