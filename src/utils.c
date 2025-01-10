#include "slice.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tokenize_user_input(Slice *s, char *input) {
  for (char *p = strtok(input, " "); p != NULL; p = strtok(NULL, " ")) {
    append_slice(s, p);
  }
}

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
