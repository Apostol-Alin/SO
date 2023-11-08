#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	int i = 1;
	printf("Starting parent %d\n", getpid());
	while(argv[i]){
		__pid_t pid = fork();
		if(pid < 0)
			return errno;
		else if(pid == 0){
			char* arg[] = {"collatzv2", argv[i], NULL};
			execve("collatzv2", arg, NULL);
			perror(NULL);
		}
		else{
			wait(NULL);
		}
		i++;
	}
	printf("Done Parent %d Me %d\n", getppid(), getpid());
	return 0;
}
