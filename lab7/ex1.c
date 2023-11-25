#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_RESOURCES 5

int available_resources = MAX_RESOURCES;

pthread_mutex_t lock;

int decrease_count(int count){
	pthread_mutex_lock(&lock);
	if(available_resources < count){
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else
		available_resources -= count;
	printf("Got %d resources %d remaining \n", count, available_resources);
	pthread_mutex_unlock(&lock);
	return 0;
}

int increase_count(int count){
	pthread_mutex_lock(&lock);
	available_resources += count;
	printf("Released %d resources %d remaining \n",count, available_resources);
	pthread_mutex_unlock(&lock);
	return 0;
}

void* f(void* v){
	int* val = (int * ) v;
	while(decrease_count(*val));
	int rez = increase_count(*val);
	free(val);
	return NULL;
}

int main(){
	pthread_t thread[5];
	if(pthread_mutex_init(&lock, NULL)){
		perror(NULL);
		return errno;
	}
	for(int i = 1; i <= 5; i++){
		int* value = (int *) malloc(sizeof(int));
		*value = i;
		if(pthread_create(&thread[i - 1], NULL, f, value)){
			perror(NULL);
			return errno;
		}
	}
	for(int i = 0; i < 5; i++){
		if(pthread_join(thread[i], NULL)){
			perror(NULL);
			return errno;
		}
	}
	pthread_mutex_destroy(&lock);
	return 0;
}
