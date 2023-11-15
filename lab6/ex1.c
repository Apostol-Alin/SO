
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


void * reverse(void* v){
	char* arg = (char *)v;
	char* rev;
	int i = 0;
	while(arg[i])
		i++;
	rev = malloc(i);
	int j = 0;
	while(j <=i){
		rev[j] = arg[i - j - 1];
		j++;
	}
	return rev;
}

int main(int argc, char* argv[]){
	pthread_t thread;
	if (pthread_create(&thread, NULL, reverse, argv[1])){
		perror(NULL);
		return errno;
	}
	void* result;
	if(pthread_join(thread, &result)){
		perror(NULL);
		return errno;
	}
	printf("%s\n", (char *) result);
	free(result);

	return 0;
}
