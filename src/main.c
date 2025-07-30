#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ncurses.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>

#include "../include/loop.h"
#include "../include/parser.h"
#include "../include/executor.h"
#include "../include/aliases.h"
#include "../include/env.h"

int check_sshlrc();
void create_sshlrc_menu();
void load_aliases_from_sshlrc();
void free_args(char **args);
char **expand_aliases(char **args);
void update_prompt();

extern void add_alias(const char *name, const char *command);
extern char *find_alias(const char *name);
extern void free_aliases();
extern void print_aliases();

extern char *custom_ps1;
extern void set_custom_ps1(const char *ps1);
extern const char *get_custom_ps1();
extern void free_custom_ps1();

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (!check_sshlrc()) {
        create_sshlrc_menu();
    } else {
        load_aliases_from_sshlrc();
        // Removed bash sourcing of .sshlrc because custom alias syntax is incompatible
        // SeaShell handles alias loading internally
        /*
        const char *home = getenv("HOME");
        if (home) {
            char command[2048];
            snprintf(command, sizeof(command), "bash --rcfile %s/.sshlrc -i -c exit", home);
            int ret = system(command);
            if (ret == -1) {
                fprintf(stderr, "SeaShell: Failed to source .sshlrc\n");
            }
        }
        */
    }

    free_aliases();
    free_custom_ps1();
    return 0;
}

int check_sshlrc() {
    char path[1024];
    const char *home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Could not get HOME environment variable\n");
        return 1; // Assume exists to skip menu
    }
    snprintf(path, sizeof(path), "%s/.sshlrc", home);
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

void create_sshlrc_menu() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    const char *choices[] = {
        "Create default .sshlrc",
        "Exit without creating"
    };
    int n_choices = sizeof(choices) / sizeof(choices[0]);
    int choice = 0;
    int highlight = 0;
    int ch;

    while(1) {
        clear();
        mvprintw(0, 0, "Welcome to SeaShell! No .sshlrc found.");
        mvprintw(1, 0, "Use arrow keys to move, Enter to select.");
        for (int i = 0; i < n_choices; i++) {
            if (i == highlight)
                attron(A_REVERSE);
            mvprintw(i + 3, 0, "%s", choices[i]);
            if (i == highlight)
                attroff(A_REVERSE);
        }
        ch = getch();
        switch(ch) {
            case KEY_UP:
                highlight--;
                if (highlight < 0) highlight = n_choices - 1;
                break;
            case KEY_DOWN:
                highlight++;
                if (highlight >= n_choices) highlight = 0;
                break;
            case 10: // Enter key
                choice = highlight;
                break;
            default:
                break;
        }
        if (ch == 10) break;
    }

    if (choice == 0) {
        // Create default .sshlrc by copying from project .sshlrc file
        const char *home = getenv("HOME");
        if (home) {
            char src_path[1024];
            char dest_path[1024];
            snprintf(src_path, sizeof(src_path), ".sshlrc");
            snprintf(dest_path, sizeof(dest_path), "%s/.sshlrc", home);

            FILE *src = fopen(src_path, "r");
            if (!src) {
                mvprintw(n_choices + 4, 0, "Failed to open default .sshlrc: %s", strerror(errno));
            } else {
                FILE *dest = fopen(dest_path, "w");
                if (!dest) {
                    mvprintw(n_choices + 4, 0, "Failed to create .sshlrc: %s", strerror(errno));
                    fclose(src);
                } else {
                    char buffer[1024];
                    size_t n;
                    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                        fwrite(buffer, 1, n, dest);
                    }
                    fclose(src);
                    fclose(dest);
                    mvprintw(n_choices + 4, 0, ".sshlrc created at %s", dest_path);
                }
            }
        } else {
            mvprintw(n_choices + 4, 0, "HOME environment variable not set.");
        }
        mvprintw(n_choices + 5, 0, "Press any key to continue...");
        getch();
    }

    endwin();
}


char **expand_aliases(char **args) {
    printf("expand_aliases called\n");
    char *alias_command = find_alias(args[0]);
    if (!alias_command) {
        printf("No alias found for %s\n", args[0]);
        // Duplicate original args to ensure safe freeing
        int orig_len = 0;
        while (args[orig_len] != NULL) orig_len++;

        char **new_args = malloc((orig_len + 1) * sizeof(char *));
        if (!new_args) {
            fprintf(stderr, "SeaShell: allocation error\n");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < orig_len; i++) {
            new_args[i] = strdup(args[i]);
            if (!new_args[i]) {
                fprintf(stderr, "SeaShell: strdup failed\n");
                for (int j = 0; j < i; j++) {
                    free(new_args[j]);
                }
                free(new_args);
                exit(EXIT_FAILURE);
            }
        }
        new_args[orig_len] = NULL;
        return new_args;
    }
    printf("Alias found for %s: %s\n", args[0], alias_command);
    // Parse alias command into args
    char *copy = strdup(alias_command);
    if (!copy) {
        fprintf(stderr, "SeaShell: strdup failed\n");
        exit(EXIT_FAILURE);
    }
    char **alias_args = parse_line(copy);
    // Do not free alias_args strings because they point into copy buffer
    // Only free the alias_args array pointer
    int alias_len = 0;
    while (alias_args[alias_len] != NULL) alias_len++;
    int orig_len = 0;
    while (args[orig_len] != NULL) orig_len++;

    int extra_args = orig_len - 1; // skip alias name
    int total = alias_len + extra_args;

    char **new_args = malloc((total + 1) * sizeof(char *));
    if (!new_args) {
        fprintf(stderr, "SeaShell: allocation error\n");
        free_args(alias_args);
        free(copy);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < alias_len; i++) {
        new_args[i] = strdup(alias_args[i]);
        if (!new_args[i]) {
            fprintf(stderr, "SeaShell: strdup failed\n");
            for (int j = 0; j < i; j++) {
                free(new_args[j]);
            }
            free(new_args);
            free_args(alias_args);
            free(copy);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < extra_args; i++) {
        new_args[alias_len + i] = strdup(args[i + 1]);
        if (!new_args[alias_len + i]) {
            fprintf(stderr, "SeaShell: strdup failed\n");
            for (int j = 0; j < alias_len + i; j++) {
                free(new_args[j]);
            }
            free(new_args);
            free_args(alias_args);
            free(copy);
            exit(EXIT_FAILURE);
        }
    }
    new_args[total] = NULL;

    free_args(alias_args);
    free(copy);
    return new_args;
}

void free_args(char **args) {
    printf("free_args called\n");
    if (!args) return;
    for (int i = 0; args[i] != NULL; i++) {
        printf("freeing arg %d: %s\n", i, args[i]);
        if (args[i] == NULL) {
            fprintf(stderr, "free_args: NULL pointer at index %d\n", i);
        } else {
            free(args[i]);
        }
    }
    free(args);
}

void free_args_array_only(char **args) {
    if (!args) return;
    free(args);
}

void load_aliases_from_sshlrc() {
    const char *home = getenv("HOME");
    if (!home) return;
    char path[1024];
    snprintf(path, sizeof(path), "%s/.sshlrc", home);
    FILE *f = fopen(path, "r");
    if (!f) return;

    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') continue;
        // Check if line starts with "alias "
        if (strncmp(line, "alias ", 6) == 0) {
            char *alias_def = line + 6;
            // Parse alias name and command separated by space
            char *name = strtok(alias_def, " \t\r\n");
            char *command = strtok(NULL, "\r\n");
            if (!name || !command) continue;
            // Remove trailing newline from command if present
            size_t len = strlen(command);
            if (len > 0 && command[len - 1] == '\n') {
                command[len - 1] = '\0';
            }
            printf("Loading alias: %s='%s'\n", name, command);
            add_alias(name, command);
        }
        // Check if line starts with "PS1="
        else if (strncmp(line, "PS1=", 4) == 0) {
            char *ps1_val = line + 4;
            // Remove trailing newline
            size_t len = strlen(ps1_val);
            if (len > 0 && ps1_val[len - 1] == '\n') {
                ps1_val[len - 1] = '\0';
            }
            free(custom_ps1);
            custom_ps1 = strdup(ps1_val);
        }
    }
    fclose(f);
}

void update_prompt() {
    if (custom_ps1) {
        printf("%s", custom_ps1);
    } else {
        printf("SeaShell> ");
    }
}
