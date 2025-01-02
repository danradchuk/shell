#include "slice.h"
#include "utils.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_pid_t.h>
#include <unistd.h>

pid_t fg_pid = -1;

static void handle_sigint(int sig) {
  if (fg_pid > 0) { // there is a foreground task
    // Kill a child's process group if it's running
    kill(-fg_pid, SIGINT);
  } else {
    write(STDOUT_FILENO, "\nsh:$ ", 6);
  }
}

int main(int argc, char **argv) {
  // handle sigint
  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  for (;;) {
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

    // parse user input
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

    // fork & exec
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (pid == 0) {
      setpgid(0, 0);
      signal(SIGINT, SIG_DFL);

      if (execvp(cmd, (&slice)->data) == -1) {
        perror("execvp failed");
        exit(1);
      }
    } else {
      fg_pid = pid;

      int status;
      int pid;

      do {
        pid = waitpid(fg_pid, &status, WUNTRACED | WCONTINUED);

        if (pid == -1) {
          perror("waitpid");
          exit(EXIT_FAILURE);
        }

      } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      fg_pid = -1;
    }

    free(input);
    dispose_slice(&slice);
  }

  return 0;
}
