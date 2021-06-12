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

void print_status(int n, char **tasks, char ***config_file, int *requests){
	for(int i = 0; i < n; i++){
		if(tasks[i] != NULL)
			printf("task #%d: %s\n", i, tasks[i]);
	}
	for(int i = 0; i < NR_FILTERS; i++){
		printf("filter %s: %d/%s (running/max)\n", config_file[i][0], requests[i], config_file[i][2]);
	}
	printf("pid: %d\n", getpid());
}

void fill_request_values(int *requests){
	for(int i = 0; i < NR_FILTERS; i++)
		requests[i] = 0;	
}

int main(int argc, char *argv[]){
	signal(SIGINT, sigINT_handler);
	char buf[BUFFER_SIZE];
	int nTasks = 4;
	int requests[NR_FILTERS]; fill_request_values(requests);
	char **tasks = malloc(sizeof(char *) * nTasks);

	if(argc == 3 && argsAreValid(argv)){
		char ***config_file = readFilters(argv[1]);
		//print_status(nTasks, tasks, config_file, requests);

		if(mkfifo("99_fifo", 0666) == -1)
			perror("mkfifo");

		int main_fifo = open("99_fifo", O_RDONLY);
		while(1){
			while(read(main_fifo, buf, BUFFER_SIZE) > 0){
				printf("%s\n", buf);
				memset(buf, 0, BUFFER_SIZE);
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
