#ifndef JOB_H
#define JOB_H

#include <sys/types.h>

#define MAX_JOB 100

typedef struct Job {
  size_t idx;
  char *cmd;
  pid_t pid;
  char *status;
  int is_background;
} Job;

void add_job(Job *jobs[], size_t *jobs_count, pid_t pid, char *cmd,
             int is_background);
void delete_job(Job *jobs[], size_t *jobs_count, pid_t pid);
void change_job_status(Job *jobs[], pid_t pid, char *status);
Job *get_job_by_pid(Job *jobs[], pid_t pid);
void list_job(Job *jobs[], size_t jobs_count);

#endif // JOB_H
