#ifndef ALIASES_H
#define ALIASES_H

void add_alias(const char *name, const char *command);
char *find_alias(const char *name);
void free_aliases();
void print_aliases();

#endif // ALIASES_H
