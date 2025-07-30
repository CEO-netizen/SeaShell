#define _POSIX_C_SOURCE 200809L
#include "env.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *custom_ps1 = NULL;

void set_custom_ps1(const char *ps1) {
    if (custom_ps1) {
        free(custom_ps1);
    }
    custom_ps1 = strdup(ps1);
}

const char *get_custom_ps1() {
    return custom_ps1;
}

void free_custom_ps1() {
    if (custom_ps1) {
        free(custom_ps1);
        custom_ps1 = NULL;
    }
}
