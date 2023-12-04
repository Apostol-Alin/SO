#include <semaphore.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

int n;
int value;

pthread_mutex_t lock;
sem_t sem;

void init_barrier(int x){
	pthread_mutex_init(&lock, NULL);
	sem_init(&sem, 0, 0);
	// initializam semaforul cu 0
	// astfel fortam toate thread-urile sa se puna la coada si sa astepte
	// sa ajunga si "ultimul" thread la bariera
	// sa se poata elibera toate
	value = 0;
	n = x;
}

void barrier_point(){
	pthread_mutex_lock(&lock);
	value += 1;
	// cand un thread ajunge la bariera
	// marcheaza acest lucru si se verifica daca este ultimul asteptat
	if(value == n){
		for(int i = 0; i < n; i++)
			sem_post(&sem);
		// in acest caz sem_post incrementeaza semaforul cu 1
		// si marcheaza ca se poate elibera un thread care astepta la coada
	}
	pthread_mutex_unlock(&lock);
	sem_wait(&sem);
}

void *tfun(void* v){

	int *tid = (int *) v;
	printf("%d reached the barrier\n", *tid);
	barrier_point();
	printf("%d passed the barrier\n", *tid);
	free(tid);
	return NULL;
}

int main(){
	pthread_t thread[5];
	init_barrier(5);
	for(int i = 0; i < 5; i++){
		int * tid = (int *) malloc(sizeof(int));
		*tid = i;
		if(pthread_create(&thread[i], NULL, tfun, tid)){
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
	sem_destroy(&sem);
	return 0;
}
