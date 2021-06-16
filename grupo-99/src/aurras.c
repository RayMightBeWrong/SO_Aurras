#include "../include/auxAurras.h"


int main(int argc, char *argv[]){
	int process = process_args(argc, argv);
	char *errorArgs = "Argumentos não reconhecidos!\n";
	char *errorFilters = "Filtros não reconhecidos!\n";
	char *outInEquals = "Os ficheiros de input e de output têm que ser diferentes!\n";
	
	switch(process){
		case -1: write(1, errorArgs, strlen(errorArgs));
			 break;

		case  0: print_info(argv[0]);
			 break;
		
		case  1: exe_status(argc, argv); 
			 break;

		case  2: if(strcmp(argv[2], argv[3]) == 0)
				 write(1, outInEquals, strlen(outInEquals));
			 else{
				 if(filtersAreValid(argc, argv))
			 		exe_transform(argc, argv);
				 else
					write(1, errorFilters, strlen(errorFilters));
			 }
			 break;
	}

	return 0;
}
