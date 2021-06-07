#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define BUFFER 1024

int main(){
	int firstToOpenMainFifo = 0;

	if(mkfifo("fifo", 0666) == -1)
		firstToOpenMainFifo = 1;

	int res;
	int file = open("fifo", O_WRONLY);
	char buf[BUFFER];

	while((res = read(0, buf, BUFFER)) > 0){
		if(strcmp(buf, "exit\n") == 0)
			break;
		write(file, buf, res);
	}
        
	close(file);
	return 0;
}
