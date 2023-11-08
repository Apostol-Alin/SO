#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	printf("%d: ",n);
	collatz(n);
	printf("Child %d finished\n",getpid());
	return 0;
}
