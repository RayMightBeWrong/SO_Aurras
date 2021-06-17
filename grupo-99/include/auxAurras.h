#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

int args_len(int argc, char *argv[]);
char* request_to_server(int argc, char *argv[]);
char* getpid_string();

int process_args(int argc, char *argv[]);
int filterIsValid(char *filter);
int filtersAreValid(int argc, char *argv[]);
int starts_with_pid(char *arg);
int is_status_last_line(char *arg);

void print_info(char *arg);
int exe_status(int argc, char *argv[]);
int exe_transform(int argc, char *argv[]);
