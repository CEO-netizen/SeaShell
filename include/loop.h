#ifndef LOOP_H
#define LOOP_H

void shell_loop();
void update_prompt();
char *read_line();
char **parse_line(char *line);
char **expand_aliases(char **args);
int execute(char **args);
void free_args(char **args);
void free_args_array_only(char **args);

#endif // LOOP_H
