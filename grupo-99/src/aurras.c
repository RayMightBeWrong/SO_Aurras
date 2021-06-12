#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER 1024

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
	char *s = malloc(sizeof(char) * args_len(argc, argv));

	int total = 0;
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

int main(int argc, char *argv[]){
	int main_fifo = open("99_fifo", O_WRONLY);

	if(process_args(argc, argv) != -1){
		char* request = request_to_server(argc, argv);
		write(main_fifo, request, strlen(request));
	}
	else{
		char *error = "Argumentos não reconhecidos\n";
		write(1, error, strlen(error));
	}
        
	close(main_fifo);
	return 0;
}
