#include "../include/auxAurrasd.h"


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
				int client_fifo = open(getpid_arg(buf), O_WRONLY);
				
				if(!process){
					if(fork() == 0){
						dup2(client_fifo, 1);
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
					write(client_fifo, "start", 5);
					make_transform(args, size, config_file, argv[2]);
					write(client_fifo, "done", 4);
				}

				memset(buf, 0, BUFFER_SIZE);
				close(client_fifo);
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
