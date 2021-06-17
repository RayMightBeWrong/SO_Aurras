#include "../include/auxAurrasd.h" 

void sigINT_handler(int signum){
	unlink("99_fifo");
	kill(getpid(), SIGKILL);
}

void sigALRM_handler(int signum){
        alarm(1);
	seconds++;
}

void sigTERM_handler(int signum){
	while(active_tasks(pid_tasks)){
		int status;
		pid_t terminated = wait(&status);
		int index = search_pid(pid_tasks, terminated);
		char *client_fifo = malloc(sizeof(char) * 16);
		sprintf(client_fifo, "%d", fifos[index]);
		int fd = open(client_fifo, O_WRONLY);
		write(fd, "maybe?", 6);
		close(fd);
	}
	unlink("99_fifo");
	kill(getpid(), SIGKILL);
}

ssize_t readln(int fd, char* line, size_t size){
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

// Separa as palavras de um pedido
// (separadas por um espaço)
char** word_separator(char *request, int *size){
        int n = 3;
        char **args = malloc(sizeof(char *) * n);

        int i, lastSpace = 10, nr_args = 0;
        for(i = 10; request[i] != '\0'; i++){
                if(request[i] == ' '){
                        if(nr_args == n){
                                args = increase_string_array(args, n);
				n += 2;
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

int isBarrier(char *request){
	int r = 1;

	if(request[0] != '(') r = 0;
	if(request[1] != '-') r = 0;
	if(request[2] != '_') r = 0;
	if(request[3] != '-') r = 0;
	if(request[4] != ')') r = 0;

	return r;
}

// Conta o número de palavras num pedido
int request_counter(char *request){
	int counter = 1;

	for(int i = 0; request[i] != '\0'; i++){
		if(request[i] == '('){
			if(isBarrier(request + i)){
				i += 5;
				if(request[i] != '\0')
					counter ++;
			}
		}
	}

	return counter;
}

// Divide o que é lido do pipe em vários pedidos
char** split_multiple_requests(char *request, int n){
	char **r = malloc(sizeof(char *) * n);
	int end, ini = 0;

	for(int i = 0; i < n; i++){
		r[i] = rm_until_barrier(request + ini, ini, &end);
		ini += end;
	}
	
	return r;
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

// Lê o ficheiro de configuração com os filtros e a sua capacidade máxima
char*** readFilters(char *file){
        char ***config_file = malloc(sizeof(char **) * NR_FILTERS);
        char buf[BUFFER_SIZE];
        memset(buf, 0, BUFFER_SIZE);
        int fd = open(file, O_RDONLY);

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

// Retorna o pid no início de um pedido
char* getpid_arg(char *arg){
        char *pid = malloc(sizeof(char) * 8);

        int i;
        for(i = 0; arg[i] != '\0' && arg[i] != '_'; i++)
                pid[i] = arg[i];

        pid[i] = '\0';

        return pid;
}

void fill_with_zero(int *v, int size){
        for(int i = 0; i < size; i++)
                v[i] = 0;
}

// Remove o pid no início de um pedido
char* rm_pid_from_request(char *request){
        int i;
        for(i = 0; request[i] != '_'; i++);
        i++;

        char *copy = malloc(sizeof(char) * strlen(request));
        for(int j = 0; request[j + i] != '\0'; j++)
                copy[j] = request[j + i];

        return copy;
}

// Retorna o conteudo de um pedido até ao separador "(-_-)"
char* rm_until_barrier(char *request, int ini, int *iniNext){
        int i;
	int bool = 0;
        char *copy = malloc(sizeof(char) * strlen(request));
        for(i = 0; !bool && request[i] != '\0'; i++){
		if(request[i] == '(')		
			if(isBarrier(request + i)){
				bool = 1;
				*iniNext = i + 5;
			}
			else
                		copy[i] = request[i];
		else
			copy[i] = request[i];
	}
	copy[i] = '\0';

        return copy;
}

// Retorna o executável e a posição no ficheiro em que se encontra um certo filtro
char* match_filter(char ***config_file, char *filter, int *pos){
        int bool = 0;
        char *r;
	*pos = -1;

        for(int i = 0; !bool && i < NR_FILTERS; i++){
                if(strcmp(filter, config_file[i][0]) == 0){
                        bool = 1;
                        r = malloc(sizeof(char) * strlen(config_file[i][1]));
                        strcpy(r, config_file[i][1]);
			*pos = i;
                }
        }

        return r;
}

// Adiciona o path da pasta ao executável
char* join_path_to_filter(char *dir, char *filter){
        char *r = malloc(sizeof(char) * (strlen(dir) + 2 + strlen(filter)));

        int i;
        for(i = 0; dir[i] != '\0'; i++)
                r[i] = dir[i];
        r[i++] = '/';
        for(int j = 0; filter[j] != '\0'; i++, j++)
                r[i] = filter[j];
        r[i] = '\0';

        return r;
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

// Verifica se a pasta dos argumentos contém os filtros
int folderHasFilters(char *folder){
	int r;
	int eco = open(join_path_to_filter(folder, "aurrasd-echo"), O_RDONLY);
	int lento = open(join_path_to_filter(folder, "aurrasd-tempo-half"), O_RDONLY);
	int baixo = open(join_path_to_filter(folder, "aurrasd-gain-half"), O_RDONLY);
	int alto = open(join_path_to_filter(folder, "aurrasd-gain-double"), O_RDONLY);
	int rapido = open(join_path_to_filter(folder, "aurrasd-tempo-double"), O_RDONLY);

	if(eco == -1 || lento == -1 || baixo == -1 || alto == -1 || rapido == -1)
		r = 0;
	else
		r = 1;

	close(eco); close(lento); close(baixo); close(alto); close(rapido);
	return r;
}

// Decide o que fazer com o pedido
int process_request(char *request){
        if(strcmp("status", request) == 0) return 0;
        else                               return 1;
}

// Gere a capacidade dos filtros existentes
void manage_requests(int *requests, char **args, int size, char ***config_file, int addOrSub){
	for(int i = 2; i < size; i++){
		int ind;
		match_filter(config_file, args[i], &ind);
		if(addOrSub)
			requests[ind]++;
		else
			requests[ind]--;
	}	
}

// Verifica se há capacidade para executar o pedido naquele momento
int capacity_to_exe(int *requests, char **args, int size, char ***config_file){
	int r = 1;

	for(int i = 2; r && i < size; i++){
		int ind;
		char *freeS = match_filter(config_file, args[i], &ind);
		if(requests[ind] + 1 > atoi(config_file[ind][2]))
			r = 0;
		free(freeS);
	}	

	return r;
}

// Verifica se há capacidade para executar o pedido mesmo se não houver outros pedidos
int capacity_to_exe_at_all(int *requests, char **args, int size, char ***config_file){
	int r = 1;
	int *tmpReq = malloc(sizeof(int) * NR_FILTERS);
	fill_with_zero(tmpReq, NR_FILTERS);

	for(int i = 2; r && i < size; i++){
		int ind;
		match_filter(config_file, args[i], &ind);
		tmpReq[ind]++;
		if(tmpReq[ind] > atoi(config_file[ind][2]))
			r = 0;
	}	

	return r;
}

// Verifica se o servidor está a usar todos os processos
int max_tasks(int *pid_tasks){
	int r = 1;

	for(int i = 0; r && i < MAX_FORKS; i++)
		if(pid_tasks[i] == 0)
			r = 0;

	return r;
}

int search_free(int *pids){
	int r = -1;

	for(int i = 0; r == -1 && i < MAX_FORKS; i++)
		if(pids[i] == 0)
			r = i;

	return r;	
}

int search_pid(int *pids, int pid){
	int r = -1;

	for(int i = 0; r == -1 && i < MAX_FORKS; i++)
		if(pid == pids[i])
			r = i;
	
	return r;
}

// Verifica se há algum processo ativo
int active_tasks(int *pids){
	int r = 0;

	for(int i = 0; !r && i < MAX_FORKS; i++)
		if(pids[i] != 0)
			r = 1;

	return r;
}

// Gere o final de uma execução
char* wipe_pid(int *pid_tasks, char **tasks, int pid){
	char *r = NULL;
	int index = search_pid(pid_tasks, pid);	
	
	if(index != -1){
		r = malloc(sizeof(char) * BUFFER_SIZE);
		strcpy(r, tasks[index]);
		tasks[index] = NULL;
		pid_tasks[index] = 0;
		fifos[index] = 0;
	}

	return r;
}

void wait_for_child(int *pid_tasks, char **tasks, int pid, char ***config_file, int *requests, int *isThereChild){
	int status;
	pid_t terminated = wait(&status);
	if(terminated == -1) *isThereChild = 0;
	else{
		char *tmp = wipe_pid(pid_tasks, tasks, terminated);
		int size;
		if(tmp != NULL){
			char **args = word_separator(tmp, &size);
			manage_requests(requests, args, size, config_file, 0);
		}
	}
}

void print_status(int n, char **tasks, int *nr_tasks, char ***config_file, int *requests, int pid){
        for(int i = 0; i < n; i++){
                if(tasks[i] != NULL)
                        printf("task #%d: %s\n", nr_tasks[i], tasks[i]);
        }
        for(int i = 0; i < NR_FILTERS; i++){
                printf("filter %s: %d/%s (running/max)\n", config_file[i][0], requests[i], config_file[i][2]);
        }
        printf("pid: %d\n", pid);
}

int simple_transform(char **args, char ***config_file, char *dir){
        int input = open(args[0], O_RDONLY, 0666);
        int output = open(args[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);

	if(input == -1 || output == -1) return -1;
        if(fork() == 0){
                dup2(input, 0);
                close(input);
                dup2(output, 1);
                close(output);
		int tmp;
                char *exe = join_path_to_filter(dir, match_filter(config_file, args[2], &tmp));
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
	int return_value = 0;
        int nr_forks = size - 3;
        if(nr_forks == 0)
                return_value = simple_transform(args, config_file, dir);
        else{
                int input = open(args[0], O_RDONLY, 0666);
                int output = open(args[1], O_CREAT | O_TRUNC | O_WRONLY, 0666);
		
		if(input == -1 || output == -1)	return -1;

                int pipes[nr_forks][2];
                for(int i = 0; i < nr_forks; i++){
                        int r = pipe(pipes[i]);
                        if(fork() == 0 && r != -1){
                                if(i == 0){
                                        close(pipes[i][0]);
                                        dup2(input, 0);
                                        dup2(pipes[i][1], 1);
                                        close(input);
                                        close(pipes[i][1]);
                                }
                                else{
                                        close(pipes[i][0]);
                                        dup2(pipes[i-1][0], 0);
                                        dup2(pipes[i][1], 1);
                                        close(pipes[i-1][0]);
                                        close(pipes[i][1]);
                                }
				int tmp;
				char *exe = join_path_to_filter(dir, match_filter(config_file, args[i + 2], &tmp));
				execlp(exe, exe, NULL);
                                _exit(-1);
                        }
			else{
				if(r != -1){
				       	close(pipes[i][1]); 
				       	int status;
				   	pid_t terminated = wait(&status);
					int exit_value = WEXITSTATUS(status);
					if(terminated == -1 || exit_value == 255){
						return_value = -1;
						break;
					}
				}
				else{
					return_value = -1;
					break;
				}
                        }
                }
		if(return_value == 0){
			if(fork() == 0){
				dup2(pipes[nr_forks - 1][0], 0);
				dup2(output, 1);
				close(pipes[nr_forks - 1][0]);
				close(output);
				int tmp;
	    			char *exe = join_path_to_filter(dir, match_filter(config_file, args[size - 1], &tmp));
	    			execlp(exe, exe, NULL);
	    			_exit(-1);
	    		}
	     		else{
	      			int status;
				pid_t terminated = wait(&status);
				int exit_value = WEXITSTATUS(status);
				if(terminated == -1 || exit_value == 255){
					return_value = -1;
				}
			}
		}
        }

        return return_value;
}

void exe_status(int client_fifo, int nTasks, char **tasks, int *nr_tasks, char ***config_file, int *requests, int pid){
	int fork_ret;
	if((fork_ret = fork()) == 0){
		dup2(client_fifo, 1);
		print_status(nTasks, tasks, nr_tasks, config_file, requests, pid);
		_exit(0);
	}
	else{
		int status;
		waitpid(fork_ret, &status, 0);
	}
}
