#include "slice.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <unistd.h>

int main(int argc, char **argv) {
  for (;;) {
    // 1. Wait for a user input
    printf("> ");
    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    read = getline(&input, &len, stdin);
    if (read == -1) {
      perror("getline");
      return 1;
    }

    remove_trailing_whitespaces(input);

    // 2. Parse the user input
    //    ls    -lh              /etc
    //    |      |                |
    // command  options (opt.)  argument (opt.)

    // extractCommand(0) -> offset for options
    // extractOptions(offset for options) -> offset for arguments
    // extractArguments(offset for arauments)

    Slice slice;
    init_slice(&slice, 4);

    tokenize_user_input(&slice, input);

    print_slice(&slice);

    char *cmd = get_slice_elem(&slice, 0);
    append_null(&slice); // required for calling execvp

    // 3. fork & exec
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (pid == 0) {
      if (execvp(cmd, (&slice)->data) == -1) {
        perror("execvp failed");
        exit(1); // Exit if execvp fails
      }
    } else {
      wait(NULL);
    }

    dispose_slice(&slice);
  }

  return 0;
}
