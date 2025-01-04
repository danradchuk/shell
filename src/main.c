#include "slice.h"
#include "utils.h"
#include <_string.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_JOB 100

typedef struct Job {
  char *cmd;
  pid_t pid;
  char *status;
} Job;

// Global State
Job *jobs[MAX_JOB];
int jobs_count = 0;
volatile sig_atomic_t pid;
volatile sig_atomic_t fg_pid;

static void add_job(pid_t pid, char *cmd) {
  if (jobs_count >= MAX_JOB - 1) {
    write(STDOUT_FILENO, "Max jobs count has reached.\n", 29);
    return;
  }

  Job *j = malloc(sizeof(Job));
  if (j == NULL) {
    err(EXIT_FAILURE, "malloc");
  }

  j->cmd = cmd;
  j->pid = pid;
  j->status = "running";

  if (jobs[jobs_count] != NULL) {
    for (int i = 0; i < MAX_JOB; i++) { // find a next free slot for the job
      if (jobs[i] == NULL) {
        jobs[jobs_count] = j;
      }
    }
  } else {
    jobs[jobs_count] = j;
  }

  jobs_count++;
}

static void delete_job(pid_t pid) {
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL && jobs[i]->pid == pid) {
      free(jobs[i]);
      jobs_count--;
    }
  }
}

static void change_job_status(pid_t pid, char *status) {
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL && jobs[i]->pid == pid) {
      jobs[i]->status = status;
    }
  }
}

static void list_job() {
  if (jobs_count == 0) {
    printf("jobs: There are no jobs\n");
    return;
  }

  // int spaces = 4;
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL) {
      printf("PID   COMMAND    STATUS\n");
      printf("%d    %s          %s\n", jobs[i]->pid, jobs[i]->cmd,
             jobs[i]->status);
    }
  }
}

static void handle_sigcont(int sig) {}

static void handle_sigint(int sig) {
  // char buf[256];
  // int len = snprintf(buf, sizeof(buf), "\n[%d]\n", fg_pid);
  // write(STDOUT_FILENO, buf, len);

  if (fg_pid > 0) { // there is a foreground task
    // Kill a child's process group if it's running
    kill(-fg_pid, SIGINT);
  } else {
    write(STDOUT_FILENO, "\nsh:$ ", 6);
  }
}

static void handle_sigtstp(int sig) {
  if (fg_pid > 0) { // there is a foreground task
    // Kill a child's process group if it's running
    kill(-fg_pid, SIGTSTP);
  } else {
    write(STDOUT_FILENO, "\nsh:$ ", 6);
  }
}

static void handle_sigchld(int sig) {
  // pid = waitpid(-1, NULL, 0); // must be while loop to reap all children
  // // printf("%d\n", pid);
  // if (pid != fg_pid) {
  //   char buf[256];
  //   int len = snprintf(buf, sizeof(buf), "\n[%d] has ended.\n", pid);
  //   write(STDOUT_FILENO, buf, len);
  //   write(STDOUT_FILENO, "sh:$ ", 5);
  // }

  int status;
  pid_t waited_pid;

  while ((waited_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    if (WIFSTOPPED(status)) {
      change_job_status(waited_pid, "stopped");
      write(STDOUT_FILENO, "sh:$ 1", 5);
      return;
    }

    if (waited_pid != fg_pid) { // background job
      char buf[256];
      int len = snprintf(buf, sizeof(buf), "\nsh: Job, [%d] '%s' has ended.\n",
                         waited_pid, "sleep");
      write(STDOUT_FILENO, buf, len);
      write(STDOUT_FILENO, "sh:$ ", 5);
      delete_job(waited_pid);
    }
    pid = waited_pid;
  }
}

int main(int argc, char **argv) {
  int bg;
  sigset_t new_mask, old_mask;

  // handle SIGINT
  if (signal(SIGINT, handle_sigint) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  // handle SIGCHLD
  if (signal(SIGCHLD, handle_sigchld) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  // handle SIGTSTP
  if (signal(SIGTSTP, handle_sigtstp) == SIG_ERR) {
    perror("signal");
    exit(1);
  }

  sigemptyset(&new_mask);
  sigaddset(&new_mask, SIGCHLD);

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
    char *amp = get_slice_elem(&slice, (&slice)->len - 1);

    // handle background job sign
    if (strcmp(amp, "&") == 0) {
      bg = 1;
      (&slice)->data[(&slice)->len - 1] = NULL;
    } else {
      bg = 0;
      append_null(&slice); // required for calling execvp
    }

    // handle exit
    if (strcmp(cmd, "exit") == 0) {
      exit(0);
    }

    // handle jobs list
    if (strcmp(cmd, "jobs") == 0) {
      list_job();
      continue;
    }

    // handle kill -CONT
    if (strcmp(cmd, "kill") == 0) {
      if ((&slice)->len != 0) {
        char *opt = get_slice_elem(&slice, 1);
        if (strcmp(opt, "-CONT") == 0) {
          char *pid = get_slice_elem(&slice, 2);
          change_job_status(atoi(pid), "running");
          continue;
        }
      }
    }

    // fork & exec
    if (!bg) {
      sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    }

    pid_t c_pid = fork();
    if (c_pid < 0) {
      perror("fork failed");
      return 1;
    }

    if (c_pid == 0) {
      setpgid(0, 0); // create a new process group with PGID = c_pid
      signal(SIGINT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      if (execvp(cmd, (&slice)->data) == -1) {
        free(input);
        dispose_slice(&slice);

        perror("execvp failed");
        exit(1);
      }
    } else {
      if (!bg) {
        fg_pid = c_pid;
        printf("Parent: %d\n", fg_pid);
        pid = 0;
        while (!pid) {
          sigsuspend(&old_mask);
        }
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
      } else {
        char *str = strdup(cmd);
        add_job(c_pid, str);
        printf("%d %s has started\n", c_pid, strdup(str));
      }
    }

    free(input);
    dispose_slice(&slice);
  }

  return 0;
}
