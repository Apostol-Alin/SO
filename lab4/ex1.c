#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

int main(){
	__pid_t pid = fork();
	if(pid < 0)
		return errno;
	else if (pid == 0){
		printf("My PID=%d, Child PID=%d\n",getppid(), getpid());
		char* argv[] = {"ls", "-l", NULL};
		execve("/usr/bin/ls", argv, NULL);
		perror(NULL);
	}
	else{
		wait();
	}
	return 0;
}
