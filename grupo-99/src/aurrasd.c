#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define NR_FILTERS 5

void sigINT_handler(int signum){	
	unlink("99_fifo");
	kill(getpid(), SIGKILL);
}

ssize_t readln(int fd, char* line, size_t size) {
	int i = 0, bool = 1;

	while(i < size - 1 && bool) {
 		ssize_t byte = read(fd, &line[i], 1);
	 	if(byte > 0){ 
			if(line[i++] == '\n') 
				bool = 0;
		}
		else
			bool = 0;
	}
	line[i] = '\0';
    
	return i;
}

char** readFilter(char *line){
	char **ret = malloc(sizeof(char *) * 3);
	char *word = malloc(sizeof(char) * 24);

	int i, res, retI;
	i = res = retI = 0;
	for(i = 0; line[i] != '\0'; i++){
		if(line[i] != '\n' && line[i] != ' '){
			word[res++] = line[i];
		}
		else{
			word[res] = '\0';
			ret[retI] = malloc(sizeof(char) * res);
			strcpy(ret[retI], word);
			retI++;
			res = 0;
		}
	}

	return ret;
}


char*** readFilters(char *file){
	char ***config_file = malloc(sizeof(char **) * NR_FILTERS);
	char buf[BUFFER_SIZE];
	memset(buf, 0, BUFFER_SIZE);
	int fd = open(file, O_RDONLY);

	ssize_t res;
	int i = 0;
	while(i < NR_FILTERS){
		if (readln(fd, buf, BUFFER_SIZE) > 0){
			config_file[i] = readFilter(buf);
			memset(buf, 0, BUFFER_SIZE);
		}
		else break;
		i++;
	}

	close(fd);
	return config_file;
}

int argsAreValid(char *argv[]){
	int r = 1;
	
	int file = open(argv[1], O_RDONLY);
	if (file == -1)
		r = 0;
	else
		close(file);
	
	return r;
}

void print_status(int n, char **tasks, char ***config_file, int *requests, int pid){
	for(int i = 0; i < n; i++){
		if(tasks[i] != NULL)
			printf("task #%d: %s\n", i, tasks[i]);
	}
	for(int i = 0; i < NR_FILTERS; i++){
		printf("filter %s: %d/%s (running/max)\n", config_file[i][0], requests[i], config_file[i][2]);
	}
	printf("pid: %d\n", pid);
}

void fill_request_values(int *requests){
	for(int i = 0; i < NR_FILTERS; i++)
		requests[i] = 0;	
}

char* getpid_arg(char *arg){
	char *pid = malloc(sizeof(char) * 8);
	
	int i;
	for(i = 0; arg[i] != '\0' && arg[i] != '_'; i++)
		pid[i] = arg[i];

	pid[i] = '\0';

	return pid;
}

void sigALRM_handler(int signum){
	alarm(1);
}

char* rm_pid_from_request(char *request){
	int i;	
	for(i = 0; request[i] != '_'; i++);
	i++;

	char *copy = malloc(sizeof(char) * strlen(request));
	for(int j = 0; request[j + i] != '\0'; j++)
		copy[j] = request[j + i];

	return copy;
}

int process_request(char *request){
	if(strcmp("status", request) == 0) return 0;
	else				   return 1;
}

char* get_word(char *bigger_word, int ini, int end){
	char *r = malloc(sizeof(char) * (end - ini));

	int k = 0;
	for(int i = ini; i < end; i++){
		r[k] = bigger_word[i];
		k++;
	}
	r[k] = '\0';

	return r;
}

char** increase_string_array(char **args, int n){
	char **clone = malloc(sizeof(char *) * (n + 2));
	
	for(int i = 0; i < n; i++){
		clone[i] = malloc(sizeof(char) * strlen(args[i]));
		strcpy(clone[i], args[i]);
	}

	return clone;
}

char** word_separator(char *request, int *size){
	int n = 3;
	char **args = malloc(sizeof(char *) * n);

	int i, lastSpace = 10, nr_args = 0;
	for(i = 10; request[i] != '\0'; i++){
		if(request[i] == ' '){
			//testar
			if(nr_args == n){
				args = increase_string_array(args, n);
			}
			args[nr_args++] = get_word(request, lastSpace, i);
			i++;
			lastSpace = i;
		}
	}
	if(nr_args == n){
		args = increase_string_array(args, n);
		n += 2;
	}
	args[nr_args++] = get_word(request, lastSpace, i);
	
	*size = nr_args;
	return args;
}

char* join_path_to_filter(char *dir, char *filter){
	char *r = malloc(sizeof(char) * (strlen(dir) + 1 + strlen(filter)));
	
	int i;
	for(i = 0; dir[i] != '\0'; i++)
		r[i] = dir[i];
	r[i++] = '/';
	for(int j = 0; filter[j] != '\0'; i++, j++)
		r[i] = filter[j];
	r[i] = '\0';

	return r;
}

char* match_filter(char ***config_file, char *filter){
	int bool = 0;
	char *r;

	for(int i = 0; !bool && i < NR_FILTERS; i++){
		if(strcmp(filter, config_file[i][0]) == 0){
			bool = 1;
			r = malloc(sizeof(char) * strlen(config_file[i][1]));
			strcpy(r, config_file[i][1]);
		}
	}

	return r;
}

int simple_transform(char **args, char ***config_file, char *dir){
	int input = open(args[0], O_RDONLY, 0666);
	int output = open(args[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);

	if(fork() == 0){
		dup2(input, 0);
		close(input);
		dup2(output, 1);
		close(output);
		char *exe = join_path_to_filter(dir, match_filter(config_file, args[2]));
		execlp(exe, exe, NULL);
		return -1;
	}
	else{
		int status;
		wait(&status);
	}

	return 0;
}

int make_transform(char **args, int size, char ***config_file, char *dir){
	int nr_forks = size - 3;
	for(int i = 2; i < size; i++){
		char *s = match_filter(config_file, args[i]);
		printf("%s | %s | %s\n", args[i], s, join_path_to_filter(dir, s));
	}
	if(nr_forks == 0){
		simple_transform(args, config_file, dir);
	}
		
	else{
		int input = open(args[0], O_RDONLY, 0666);
		int output = open(args[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
	
		int pipes[nr_forks][2];
		for(int i = 0; i < nr_forks; i++){
			pipe(pipes[i]);
			if(fork() == 0){
				if(i == 0){
					close(pipes[i][0]);
					dup2(input, 0); 
					dup2(pipes[i][1], 1);
					close(input); 
					close(pipes[i][1]);
					char *exe = join_path_to_filter(dir, match_filter(config_file, args[i + 2]));
					execlp(exe, exe, NULL);
				}
				else{
					close(pipes[i][0]);
					dup2(pipes[i-1][0], 0); 
					dup2(pipes[i][1], 1);
					close(pipes[i-1][0]); 
					close(pipes[i][1]);
					char *exe = join_path_to_filter(dir, match_filter(config_file, args[i + 2]));
					execlp(exe, exe, NULL);
				}
				_exit(-1);
			}
			else{
				int status;
				close(pipes[i][1]);
				pid_t terminated = wait(&status);
			}
		}
		if(fork() == 0){
			dup2(pipes[nr_forks - 1][0], 0);
			dup2(output, 1);
			close(pipes[nr_forks - 1][0]); 
			close(output);
			char *exe = join_path_to_filter(dir, match_filter(config_file, args[size - 1]));
			execlp(exe, exe, NULL);
			_exit(-1);
		}
		else{
			int status;
			pid_t terminated = wait(&status);
		}
	}

	return 0;
}

int main(int argc, char *argv[]){
	signal(SIGINT, sigINT_handler);
	signal(SIGALRM, sigALRM_handler);
	char buf[BUFFER_SIZE]; memset(buf, 0, BUFFER_SIZE);
	int nTasks = 4;
	int requests[NR_FILTERS]; fill_request_values(requests);
	char **tasks = malloc(sizeof(char *) * nTasks);
	int pid = getpid();

	alarm(2);
	if(argc == 3 && argsAreValid(argv)){
		char ***config_file = readFilters(argv[1]);

		if(mkfifo("99_fifo", 0666) == -1)
			perror("mkfifo");

		int main_fifo = open("99_fifo", O_RDONLY);
		while(1){
			pause();
			if(read(main_fifo, buf, BUFFER_SIZE) > 0){
				printf("%s\n", rm_pid_from_request(buf));
				printf("%d\n", process_request(rm_pid_from_request(buf)));
				char *request = rm_pid_from_request(buf);
				int process = process_request(request);
				//int client_fifo = open(getpid_arg(buf), O_WRONLY);
				
				if(!process){
					if(fork() == 0){
		//				dup2(client_fifo, 1);
						print_status(nTasks, tasks, config_file, requests, pid);
						_exit(0);
					}
					else{
						int status;
						wait(&status);
					}
				}
				else{
					int size = 0;
					char **args = word_separator(request, &size);
					for(int i = 0; i < size; i++)
						printf("%s\n", args[i]);
					putchar('\n');
					make_transform(args, size, config_file, argv[2]);
				}

				memset(buf, 0, BUFFER_SIZE);
		//		close(client_fifo);
			}
		}

		close(main_fifo);
		unlink("99_fifo");
	}
	else{
		char *error = "Não foram inseridos argumentos corretos\n";
		write(1, error, 42);
	}

	return 0;
}
