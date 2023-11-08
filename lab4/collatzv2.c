#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void collatz(int n){

	if(n == 1)
		printf("1\n");
	else{
		printf("%d ", n);
		if(n % 2 == 0)
			collatz(n / 2);
		else
			collatz(3 * n + 1);
	}

}


int main(int argc, char* argv[]){

	int n = atoi(argv[1]);
	printf("%d: ", n);
	collatz(n);
	printf("Done Parent %d Me %d\n", getppid(), getpid());
	return 0;
}
