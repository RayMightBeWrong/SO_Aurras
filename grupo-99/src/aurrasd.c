#include "../include/auxAurrasd.h"

int main(int argc, char *argv[]){
	seconds = 1;
	signal(SIGINT, sigINT_handler);
	signal(SIGALRM, sigALRM_handler);
	signal(SIGTERM, sigTERM_handler);
	char reader[BUFFER_SIZE]; memset(reader, 0, BUFFER_SIZE);
	char buf[MAX_FORKS][BUFFER_SIZE];
	for(int i = 0; i < MAX_FORKS; i++) memset(buf[i], 0, BUFFER_SIZE);
	int requests[NR_FILTERS]; fill_with_zero(requests, NR_FILTERS);
	char **tasks = malloc(sizeof(char *) * MAX_FORKS);
	pid_tasks = malloc(sizeof(int) * MAX_FORKS); fill_with_zero(pid_tasks, MAX_FORKS);
	fifos = malloc(sizeof(int) * MAX_FORKS); fill_with_zero(fifos, MAX_FORKS);
	int *nr_task = malloc(sizeof(int) * MAX_FORKS); fill_with_zero(nr_task, MAX_FORKS);
	int pid = getpid();
	total_tasks = 0;

	alarm(2);
	if(argc == 3 && argsAreValid(argv)){
		if(!folderHasFilters(argv[2])){
			char *error = "A diretoria selecionada não contém os filtros necessários!\n";
			write(1, error, strlen(error));
		}
		else{
			char ***config_file = readFilters(argv[1]);

			if(mkfifo("99_fifo", 0666) == -1)
				perror("mkfifo");

			int main_fifo = open("99_fifo", O_RDONLY);
			while(1){
				pause();
				if(seconds % 10 == 0){
					if(active_tasks(pid_tasks)){
						int tmp;
						wait_for_child(pid_tasks, tasks, pid, config_file, requests, &tmp);
					}
				}

				if(read(main_fifo, reader, BUFFER_SIZE) > 0){
					int nr_demand = request_counter(reader);
					char **demands = split_multiple_requests(reader, nr_demand);
					
					for(int i = 0; i < nr_demand; i++){
						int free = search_free(pid_tasks);
						strcpy(buf[free], demands[i]);
						printf("Executing: %s\n", rm_pid_from_request(buf[free]));
						char *request = rm_pid_from_request(buf[free]);
						int process = process_request(request);
						int client_fifo = open(getpid_arg(buf[free]), O_WRONLY);
						fifos[free] = atoi(getpid_arg(buf[free]));
				
						if(!process){
							exe_status(client_fifo, MAX_FORKS, tasks, nr_task, config_file, requests, pid);
						}
						else{
							int size = 0;
							char **args = word_separator(request, &size);
							if(capacity_to_exe_at_all(requests, args, size, config_file)){
								int wait = 1;
								while(!capacity_to_exe(requests, args, size, config_file) && wait)
									wait_for_child(pid_tasks, tasks, pid, config_file, requests, &wait);
								if(max_tasks(pid_tasks))
									wait_for_child(pid_tasks, tasks, pid, config_file, requests, &wait);

								write(client_fifo, "start", 5);
								int fork_ret;
								if((fork_ret = fork()) == 0){
									int rValue = make_transform(args, size, config_file, argv[2]);
									if(!rValue)
										write(client_fifo, "done", 4);
									else if (rValue == -1)
										write(client_fifo, "oof", 3);
									_exit(0);
								}
								else{
									printf("pid: %d\n", fork_ret);
									manage_requests(requests, args, size, config_file, 1);
									pid_tasks[free] = fork_ret;
									tasks[free] = rm_pid_from_request(buf[free]); 
									total_tasks++;
									nr_task[free] = total_tasks;
								}
							}
							else{
								write(client_fifo, "+_+", 3);
							}
						}

						memset(reader, 0, BUFFER_SIZE);
						memset(buf[free], 0, BUFFER_SIZE);
						close(client_fifo);
					}
				}
			}

			close(main_fifo);
			unlink("99_fifo");
		}
	}
	else{
		char *error = "Não foram inseridos argumentos corretos\n";
		write(1, error, strlen(error));
	}

	return 0;
}
