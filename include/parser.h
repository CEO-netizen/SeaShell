#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

char *read_line();
char **parse_line(char *line);

#endif // PARSER_H
