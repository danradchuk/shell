#include "utils.h"
#include "slice.h"
#include "slice.c"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int extract_cmd_size(char *input) {
  char *ptr = input;

  // determine the size of the command
  int size = 0;
  while (*ptr != '\0' && !isspace((unsigned char)*ptr)) {
    size++;
    ptr++;
  }

  return size;
}

char *extract_cmd(char *input, int size) {
  if (size <= 0 || input == NULL)
    return NULL;

  char *cmd = malloc((size + 1) * sizeof(char));
  if (cmd == NULL)
    return NULL;

  for (int i = 0; i < size; i++) {
    cmd[i] = input[i]; // Copy characters from input to cmd
  }
  cmd[size] = '\0'; // Null-terminate the string

  return cmd;
}

int extract_arguments_size(char *input, int offset) {
  char *ptr = input;

  // skip to the arguments
  int i = 0;
  while (i != offset) {
    ptr++;
    i++;
  }
  printf("%d\n", i);

  // determine the size of the arguments
  int size = 0;
  while (*ptr != '\0') {
    // printf("Symbol: %c\n", *ptr);
    size++;
    ptr++;
  }

  return size - 1; // cut off the whitespace from getLine() func
}

char *extract_arguments(char *input, int size) {
  if (size <= 0 || input == NULL)
    return NULL;

  char *args = malloc((size + 1) * sizeof(char));
  if (args == NULL)
    return NULL;

  size_t str_size = strlen(input); // exclude '\0'

  args = input + str_size - size - 1;
  args[size] = '\0';

  return args;
}

void tokenize_user_input(Slice *s, char *input) {
  for (char *p = strtok(input, " "); p != NULL; p = strtok(NULL, " ")) {
    append_slice(s, p);
  }
}

// char* str = "Hello, World!   "
// skip 3 whitespaces until !, replace the last of the skipped whitespaces on
// the NULL terminate symbol
void remove_trailing_whitespaces(char *str) {
  if (str == NULL)
    return;

  size_t len = strlen(str);
  while (len > 0 && isspace((unsigned char)str[len - 1])) { // Skip
    // whitespaces until a first non-whitespace symbol
    len--;
  }

  str[len] = '\0'; // Null-terminate at the new end of the string
}
