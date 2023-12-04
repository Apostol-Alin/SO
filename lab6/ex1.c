
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
	rev = malloc(i); // alocam dinamic memoria
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
	//facem un void* in care retinem ce returneaza functia
	if(pthread_join(thread, &result)){
		perror(NULL);
		return errno;
	}
	printf("%s\n", (char *) result); // ii dam cast la char *
	free(result); // eliberam memoria

	return 0;
}
