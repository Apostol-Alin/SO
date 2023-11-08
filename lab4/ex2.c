#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>


int main(int argc, char* argv[]){

	__pid_t pid = fork();

	if(pid < 0)
		return errno;

	else if (pid == 0){
	       	char* arg[] = {"collatz", argv[1], NULL};
        	execve("collatz", arg, NULL);
        	perror(NULL);
	}
	else{
		wait();
	}
	return 0;
}
