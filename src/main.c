#include "slice.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <unistd.h>

pid_t child_pid = -1;

static void sigHandler(int sig) {
  if (child_pid > 0) {
    // Kill a child's process group if it's running
    kill(-child_pid, SIGINT);
  } else {
    write(STDOUT_FILENO, "\nsh:$ ", 6);
  }
}

int main(int argc, char **argv) {
  int status;
  pid_t w;

  for (;;) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
      perror("signal");
      exit(1);
    }

    // 1. Wait for a user input
    printf("sh:$ ");
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

    // print_slice(&slice);

    char *cmd = get_slice_elem(&slice, 0);
    append_null(&slice); // required for calling execvp

    // handle exit
    if (strcmp(cmd, "exit") == 0) {
      exit(0);
    }

    // 3. fork & exec
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (pid == 0) {
      signal(SIGINT, SIG_DFL);
      if (execvp(cmd, (&slice)->data) == -1) {
        perror("execvp failed");
        exit(1); // Exit if execvp fails
      }
    } else {
      child_pid = pid;
      // waitpid(child_pid, NULL, 0); // Wait for the child to terminate

      do {
        w = waitpid(child_pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1) {
          perror("waitpid");
          exit(EXIT_FAILURE);
        }

      } while (!WIFEXITED(status) &&
               !WIFSIGNALED(status)); // TODO should handle
                                      //  suspend too
      child_pid = -1;
    }

    dispose_slice(&slice);
  }

  return 0;
}
