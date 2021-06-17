#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define NR_FILTERS  5
#define MAX_FORKS   10

int seconds, total_tasks_before, total_tasks;
int *pid_tasks;
int *fifos;

void sigINT_handler(int signum);
void sigALRM_handler(int signum);
void sigTERM_handler(int signum);

ssize_t readln(int fd, char* line, size_t size);
char* get_word(char *bigger_word, int ini, int end);
char** increase_string_array(char **args, int n);
char** word_separator(char *request, int *size);
int isBarrier(char *request);
int request_counter(char *request);
char **split_multiple_requests(char *request, int n);

char** readFilter(char *line);
char*** readFilters(char *file);
char* getpid_arg(char *arg);
void fill_with_zero(int *requests, int size);
char* rm_pid_from_request(char *request);
char* rm_until_barrier(char *request, int ini, int *iniNext);
char* match_filter(char ***config_file, char *filter, int *pos);
char* join_path_to_filter(char *dir, char *filter);

int argsAreValid(char *argv[]);
int folderHasFilters(char *folder);
int process_request(char *request);
void manage_requests(int *requests, char **args, int size, char ***config_file, int addOrSub);
int capacity_to_exe(int *requests, char **args, int size, char ***config_file);
int capacity_to_exe_at_all(int *requests, char **args, int size, char ***config_file);
int max_tasks(int *pid_tasks);
int search_free(int *pids);
int search_pid(int *pids, int pid);
int active_tasks(int *pids);
char* wipe_pid(int *pid_tasks, char **tasks, int pid);

void wait_for_child(int *pid_tasks, char **tasks, int pid, char ***config_file, int *requests, int *isThereChild);
void print_status(int n, char **tasks, int *nr_tasks, char ***config_file, int *requests, int pid);
int simple_transform(char **args, char ***config_file, char *dir);
int make_transform(char **args, int size, char ***config_file, char *dir);
void exe_status(int client_fifo, int nTasks, char **tasks, int *nr_tasks, char ***config_file, int *requests, int pid);
