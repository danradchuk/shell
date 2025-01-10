#include "job.h"
#include "utils.h"
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
  char *cmd;
  char **args;
} ExecvpCmd;

// Global State
Job *jobs[MAX_JOB];
size_t jobs_count = 0;
volatile sig_atomic_t pid;
volatile sig_atomic_t fg_pid;

static void handle_sigint(int sig) {
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
  int status;
  pid_t waited_pid;

  while ((waited_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    Job *job = get_job_by_pid(jobs, waited_pid);

    if (job->is_background) {
      if (WIFSTOPPED(status)) {
        char buf[256];
        int len = snprintf(buf, sizeof(buf),
                           "\nsh: Job %zu, [%d] '%s' has stopped.\n", job->idx,
                           waited_pid, "sleep");
        write(STDOUT_FILENO, buf, len);
        job->status = "stopped";
        return;
      }
      if (WIFEXITED(status)) {
        char buf[256];
        int len =
            snprintf(buf, sizeof(buf), "\nsh: Job %zu, [%d] '%s' has ended.\n",
                     job->idx, waited_pid, "sleep");
        write(STDOUT_FILENO, buf, len);
        write(STDOUT_FILENO, "sh:$ ", 5);

        delete_job(jobs, &jobs_count, waited_pid);
        return;
      }
      if (WIFSIGNALED(status)) {
        int sig_num = WTERMSIG(status);
        if (sig_num == 2) { // SIGINT
          delete_job(jobs, &jobs_count, waited_pid);
        }
        return;
      }
    } else {
      pid = waited_pid; // notify the main loop
      if (WIFSTOPPED(status)) {
        char buf[256];
        int len = snprintf(buf, sizeof(buf),
                           "\nsh: Job %zu, [%d] '%s' has stopped.\n", job->idx,
                           waited_pid, "sleep");
        write(STDOUT_FILENO, buf, len);
        job->status = "stopped";
        return;
      }
      if (WIFEXITED(status)) {
        delete_job(jobs, &jobs_count, waited_pid);
        return;
      }
      if (WIFSIGNALED(status)) {
        int sig_num = WTERMSIG(status);
        if (sig_num == 2) { // SIGINT
          delete_job(jobs, &jobs_count, waited_pid);
        }
        return;
      }
    }
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

    // read & sanitize a user input
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

    // rewrite bg %id command with kill -CONT <PID>
    if (strcmp((&slice)->data[0], "bg") == 0) {
      int j_id = atoi(get_slice_elem(&slice, 1));
      Job *j = jobs[j_id];

      dispose_slice(&slice);
      init_slice(&slice, 4);

      // tokenize_user_input(&slice, input);
      append_slice(&slice, "kill");
      append_slice(&slice, "-CONT");
      char buf[6];
      int len = snprintf(buf, sizeof(buf), "%d", j->pid);
      append_slice(&slice, strdup(buf));
    }

    char *cmd = strdup(get_slice_elem(&slice, 0));
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
      list_job(jobs, jobs_count);
      continue;
    }

    // handle kill -CONT
    if (strcmp(cmd, "kill") == 0) {
      if ((&slice)->len != 0) {
        char *opt = get_slice_elem(&slice, 1);
        if (strcmp(opt, "-CONT") == 0) {
          char *pid = get_slice_elem(&slice, 2);
          change_job_status(jobs, atoi(pid), "running");
        }
      }
    }

    // fork & exec
    if (!bg) {
      sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    }
    pid_t c_pid = fork();
    if (c_pid < 0) {
      free(input);
      dispose_slice(&slice);

      perror("fork");
      return 1;
    }

    if (c_pid == 0) { // child
      setpgid(0, 0);  // create a new process group with PGID = c_pid
      signal(SIGINT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      if (execvp(cmd, (&slice)->data) == -1) {
        free(input);
        dispose_slice(&slice);

        perror("execvp");
        exit(1);
      }
    } else { // parent
      // printf("Parent: %d\n", c_pid);
      // char *str = strdup(cmd);

      if (!bg) {
        fg_pid = c_pid;
        add_job(jobs, &jobs_count, c_pid, cmd, 0);
        pid = 0;
        while (!pid) {
          sigsuspend(&old_mask);
        }
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
      } else {
        add_job(jobs, &jobs_count, c_pid, cmd, 1);
        printf("%d %s has started\n", c_pid, cmd);
      }
    }

    free(input);
    dispose_slice(&slice);
  }

  return 0;
}
