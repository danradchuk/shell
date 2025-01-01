#ifndef UTILS_H
#define UTILS_H

#include "slice.h"

int extract_cmd_size(char *input);
char *extract_cmd(char *input, int size);

int extract_arguments_size(char *input, int offset);
char *extract_arguments(char *input, int size);

void tokenize_user_input(Slice *s, char *input);

void remove_trailing_whitespaces(char *str);
#endif
