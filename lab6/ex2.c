#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

struct mystruct{
	int i, j;
}; // o structura care ne permite sa trimitem mai multe valori ca parametru

int c[50][50], a[50][50], b[50][50]; // matricele declarate global pentru
				     // a putea fi vazute de fiecare thread
int n;

void* produs(void* v){
	struct mystruct *args = (struct mystruct *) v;
	// preluam argumentul
	int i = args->i;
	int j = args->j;
	int* suma = (int *) malloc(sizeof(int));
	// alocam dinamic un int *
	*suma = 0;
	for(int k = 0; k < n; k++)
		*suma = *suma + a[i][k] * b[k][j];
	return suma;
	// returnam un int * care este convertit la void *
}

int main(int argc, char* argv[]){
	n = atoi(argv[1]);
	int i, j;
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			a[i][j] = i + j;
	for(i = 0; i < n; i++)
		for(j = 0; j < n; j++)
			b[i][j] = i + j + 1;
	pthread_t thread[50][50];
	struct mystruct* pozitii = (struct mystruct*) malloc(2500 * sizeof(struct mystruct));
	// alocam dinamic argumentele
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			pozitii[i * n + j].i = i; pozitii[i * n + j].j = j;
			if(pthread_create(&thread[i][j], NULL, produs, &pozitii[i * n + j])){
				perror(NULL);
				return errno;
			}
		}
	}
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++){
			int* rez;
			if(pthread_join(thread[i][j],(void **) &rez)){
				perror(NULL);
				return errno;
			}
			c[i][j] = *rez;
			free(rez);
		}
	}
	free (pozitii);
	for(i = 0; i < n; i++){
		for(j = 0; j < n; j++)
			printf("%d ", c[i][j]);
		printf("\n");
	}
	return 0;
}
