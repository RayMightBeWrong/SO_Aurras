#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024

//validar que ficheiro de input existe
//validar se o ficheiro de input e output têm nomes diferentes

int args_len(int argc, char *argv[]){
	int r = 0;

	for(int i = 1; i < argc; i++)
		r += (int) strlen(argv[i]);

	if(argc > 2) r += (argc - 2);

	return r;
}

char* request_to_server(int argc, char *argv[]){
	char *s = malloc(sizeof(char) * (args_len(argc, argv) + 8));
	int total = 0;

	char *pid = malloc(sizeof(char) * 8);
	sprintf(pid, "%d", getpid());
	for(int i = 0; pid[i] != '\0'; i++){
		s[total++] = pid[i];
	}
	s[total++] = '_';
	for(int i = 1; i < argc; i++){
		int k = strlen(argv[i]);
		for(int j = 0; j < k; j++){
			s[total++] = argv[i][j];
		}
		if(i != argc - 1)
			s[total++] = ' ';	
	}

	return s;
}

int process_args(int argc, char *argv[]){
	int r = -1;

	if(argc == 1) r = 0;
	if(argc == 2 && strcmp(argv[1], "status") == 0) r = 1;
	if(argc > 4 && strcmp(argv[1], "transform") == 0) r = 2;

	return r;
}

char* getpid_string(){
	char *pid = malloc(sizeof(char) * 8);
	sprintf(pid, "%d", getpid());
	return pid;
}

void print_info(char *arg){
	char *fst = " status\n";
	char *snd = " transform input-filename output-filename filter-id-1 filter-id-2 ...\n";
	write(1, arg, strlen(arg));
	write(1, fst, strlen(fst));
	write(1, arg, strlen(arg));
	write(1, snd, strlen(snd));
}

int starts_with_pid(char *arg){
	if (arg[0] == '\n' && arg[1] == 'p' && arg[2] == 'i' && arg[3] == 'd') return 1;
	else if(arg[0] == 'p' && arg[1] == 'i' && arg[2] == 'd') return 1;
	else return 0;
}

int is_status_last_line(char *arg){
	int r = 0;

	if(arg[0] == 'p') r = starts_with_pid(arg);
	for(int i = 0; arg[i] != '\0' && !r; i++){
		if(arg[i] == '\n')
			r = starts_with_pid(arg + i);
	}

	return r;
}

int exe_status(int argc, char *argv[]){
	int main_fifo = open("99_fifo", O_WRONLY);
	char buf[BUFFER_SIZE]; memset(buf, 0, BUFFER_SIZE);
	mkfifo(getpid_string(), 0666);

	char* request = request_to_server(argc, argv);
	write(main_fifo, request, strlen(request));
	int client_fifo = open(getpid_string(), O_RDONLY);

	int bool = 1;
	while(bool && read(client_fifo, buf, BUFFER_SIZE) > 0){
		write(1, buf, BUFFER_SIZE);
		if(is_status_last_line(buf))
			bool = 0;
	}

	close(client_fifo); close(main_fifo);
	unlink(getpid_string());
	return 0;
}

int exe_transform(int argc, char *argv[]){
	int main_fifo = open("99_fifo", O_WRONLY);
	char buf[BUFFER_SIZE]; memset(buf, 0, BUFFER_SIZE);
	mkfifo(getpid_string(), 0666);

	char* request = request_to_server(argc, argv);
	write(main_fifo, request, strlen(request));
//	int client_fifo = open(getpid_string(), O_RDONLY);

	//close(client_fifo); 
	close(main_fifo);
	unlink(getpid_string());
	return 0;
}

int main(int argc, char *argv[]){
	int process = process_args(argc, argv);
	char *error = "Argumentos não reconhecidos\n";
	
	switch(process){
		case -1: write(1, error, strlen(error));
			 break;

		case  0: print_info(argv[0]);
			 break;
		
		case  1: exe_status(argc, argv); 
			 break;

		case  2: exe_transform(argc, argv);
			 break;

	}

	return 0;
}
