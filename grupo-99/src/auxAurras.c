#include "../include/auxAurras.h"

int args_len(int argc, char *argv[]){
        int r = 0;

        for(int i = 1; i < argc; i++)
                r += (int) strlen(argv[i]);

        if(argc > 2) r += (argc - 2);

        return r;
}

// Retorna uma string já formatada para enviar para o servidor
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
	s[total++] = '(';
	s[total++] = '-';
	s[total++] = '_';
	s[total++] = '-';
	s[total++] = ')';
	s[total++] = '\0';

        return s;
}

// Retorna o pid do processo em string
char* getpid_string(){
        char *pid = malloc(sizeof(char) * 8);
        sprintf(pid, "%d", getpid());
        return pid;
}

// Decide o que fazer com os pedidos do utilizador
int process_args(int argc, char *argv[]){
        int r = -1;

        if(argc == 1) r = 0;
        if(argc == 2 && strcmp(argv[1], "status") == 0) r = 1;
        if(argc > 4 && strcmp(argv[1], "transform") == 0) r = 2;

        return r;
}

int filterIsValid(char *filter){
        if(strcmp(filter, "eco") == 0 ||
           strcmp(filter, "alto") == 0 ||
           strcmp(filter, "rapido") == 0 ||
           strcmp(filter, "lento") == 0 ||
           strcmp(filter, "baixo") == 0)
                return 1;
        else
                return 0;
}

int filtersAreValid(int argc, char *argv[]){
	int r = 1;

	for(int i = 4; r && i < argc; i++)
		if(!filterIsValid(argv[i]))
			r = 0;

	return r;
}

int starts_with_pid(char *arg){
        if (arg[0] == '\n' && arg[1] == 'p' && arg[2] == 'i' && arg[3] == 'd') return 1;
        else if(arg[0] == 'p' && arg[1] == 'i' && arg[2] == 'd') return 1;
        else return 0;
}

// Verifica se é a última linha na impressão do status
// para poder fechar o ciclo na função exe_status
int is_status_last_line(char *arg){
        int r = 0;

        if(arg[0] == 'p') r = starts_with_pid(arg);
        for(int i = 0; arg[i] != '\0' && !r; i++){
                if(arg[i] == '\n')
                        r = starts_with_pid(arg + i);
        }

        return r;
}

void print_info(char *arg){
        char *fst = " status\n";
        char *snd = " transform input-filename output-filename filter-id-1 filter-id-2 ...\n";
        write(1, arg, strlen(arg));
        write(1, fst, strlen(fst));
        write(1, arg, strlen(arg));
        write(1, snd, strlen(snd));
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
		memset(buf, 0, BUFFER_SIZE);
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

        char *fst = "Pending...\n", *snd = "Processing...\n";
        char *end = "Done!\n";
	char *error = "\nSomething unexpected happened. We couldn't process your request.\nWe're sorry.\n";
	char *excess = "\nThe server doesn't have the capacity to run your command.\nWe're sorry for the inconvenience.\n";
	char *maybeError = "Your request is complete. But some errors may have happened in the process.\nBe warned: final product may not be as you wished.\n";

        write(1, fst, strlen(fst));
        int client_fifo = open(getpid_string(), O_RDONLY);
        while(read(client_fifo, buf, BUFFER_SIZE) > 0){
                if(strcmp(buf, "start") == 0)
                        write(1, snd, strlen(snd));
                else if (strcmp(buf, "done") == 0){
                        write(1, end, strlen(end));
			break;
                }
		else if (strcmp(buf, "+_+") == 0){
			write(1, excess, strlen(excess));	
			break;
		}
		else if (strcmp(buf, "oof") == 0){
			write(1, error, strlen(error));	
			break;
		}
		else if (strcmp(buf, "maybe?") == 0){
			write(1, maybeError, strlen(maybeError));	
			break;
		}
                memset(buf, 0, BUFFER_SIZE);
        }

        close(client_fifo);
        close(main_fifo);
        unlink(getpid_string());
        return 0;
}
