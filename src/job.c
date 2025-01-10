// #include "job.h"
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_JOB 100

typedef struct Job {
  size_t idx;
  char *cmd;
  pid_t pid;
  char *status;
  int is_background;
} Job;

void add_job(Job *jobs[], size_t *jobs_count, pid_t pid, char *cmd,
             int is_background) {
  if (*jobs_count >= MAX_JOB - 1) {
    write(STDOUT_FILENO, "Max jobs count has reached.\n", 29);
    return;
  }

  Job *j = malloc(sizeof(Job));
  if (j == NULL) {
    err(EXIT_FAILURE, "malloc");
  }

  j->idx = *jobs_count;
  j->cmd = cmd;
  j->pid = pid;
  j->status = "running";
  j->is_background = is_background;

  if (jobs[*jobs_count] != NULL) {
    for (int i = 0; i < MAX_JOB; i++) { // find a next free slot for the job
      if (jobs[i] == NULL) {
        jobs[*jobs_count] = j;
      }
    }
  } else {
    jobs[*jobs_count] = j;
  }

  (*jobs_count)++;
}

void delete_job(Job *jobs[], size_t *jobs_count, pid_t pid) {
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL && jobs[i]->pid == pid) {
      free(jobs[i]);
      jobs[i] = NULL;
      (*jobs_count)--;
    }
  }
}

void change_job_status(Job *jobs[], pid_t pid, char *status) {
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL && jobs[i]->pid == pid) {
      jobs[i]->status = status;
    }
  }
}

Job *get_job_by_pid(Job *jobs[], pid_t pid) {
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL && jobs[i]->pid == pid) {
      return jobs[i];
    }
  }
  return NULL;
}

void list_job(Job *jobs[], size_t jobs_count) {
  if (jobs_count == 0) {
    printf("jobs: There are no jobs\n");
    return;
  }

  // int spaces = 4;
  for (int i = 0; i < MAX_JOB; i++) {
    if (jobs[i] != NULL) {
      printf("JOB    PID   COMMAND    STATUS\n");
      printf("%zu      %d    %s          %s\n", jobs[i]->idx, jobs[i]->pid,
             jobs[i]->cmd, jobs[i]->status);
    }
  }
}
