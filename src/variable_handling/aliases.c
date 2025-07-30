#define _POSIX_C_SOURCE 200809L
#include "aliases.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Alias {
    char *name;
    char *command;
    struct Alias *next;
} Alias;

static Alias *alias_list = NULL;

void add_alias(const char *name, const char *command) {
    Alias *new_alias = malloc(sizeof(Alias));
    if (!new_alias) {
        fprintf(stderr, "SeaShell: allocation error\n");
        exit(EXIT_FAILURE);
    }
    new_alias->name = strdup(name);
    new_alias->command = strdup(command);
    new_alias->next = alias_list;
    alias_list = new_alias;
}

char *find_alias(const char *name) {
    Alias *current = alias_list;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->command;
        }
        current = current->next;
    }
    return NULL;
}

void free_aliases() {
    Alias *current = alias_list;
    while (current) {
        Alias *next = current->next;
        free(current->name);
        free(current->command);
        free(current);
        current = next;
    }
    alias_list = NULL;
}

void print_aliases() {
    Alias *current = alias_list;
    while (current) {
        printf("alias %s='%s'\n", current->name, current->command);
        current = current->next;
    }
}
