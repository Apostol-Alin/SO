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
	// blocam mutex-ul pentru a fi singurii cu acces la count
	if(available_resources < count){
		// daca nu am de unde sa scad, deblochez mutex-ul si semnalez cu -1
		pthread_mutex_unlock(&lock);
		return -1;
	}
	else
		available_resources -= count;
	// altfel imi continui operatiile
	printf("Got %d resources %d remaining \n", count, available_resources);
	pthread_mutex_unlock(&lock);
	// eliberez mutex-ul
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
	// aici incepe zona critica
	while(decrease_count(*val)); // trhead-ul incearca
	// sa faca decrease count corect (adica sa aiba resurse din care sa ia)

	int rez = increase_count(*val); // apoi semnaleaza ca a eliberat o resursa
	// aici se termina zona critica
	free(val);
	return NULL;
}

int main(){
	pthread_t thread[5];
	if(pthread_mutex_init(&lock, NULL)){
		perror(NULL);
		return errno;
	}
	// initializam mutex-ul
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
	// eliberam mutex-ul
	return 0;
}
