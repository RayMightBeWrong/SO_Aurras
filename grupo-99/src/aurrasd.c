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
	char buf[BUFFER];
	int res;

	while(1){
		int file = open("fifo", O_RDONLY);

		while((res = read(file, buf, BUFFER)) > 0){
			printf("%s", buf);
			if(strcmp("exit\n", buf) == 0)
				break;
			write(log, buf, res);
			memset(buf, 0, BUFFER);
		}

		close(file);
		if(strcmp("exit\n", buf) == 0)
			break;
	}

	close(log);
	unlink("fifo");
	return 0;
}
