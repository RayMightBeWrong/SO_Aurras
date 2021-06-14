#include "../include/auxAurras.h"


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
