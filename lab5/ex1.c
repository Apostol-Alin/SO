#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

void collatz(int n, int* sh, int indice){
	if(n == 1)
		sh[indice] = 1;
	else{
		sh[indice] = n;
		if(n % 2 == 0)
			collatz(n / 2, sh, indice + 1);
		else
			collatz(3 * n + 1, sh, indice + 1);
	}
}

int main(int argc, char* argv[]){

	char shm_name[] = "/mysharedmmry";
	int shm_fd;
	int i = 1;
	shm_fd = shm_open(shm_name, O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
	// flag-ul O_CREAT : daca nu exista obiectul de memorie shm_name
	// atunci este creat
	// O_RDWR sunt drepturile de citire scriere
	// functia returneaza un descriptor
	if (shm_fd < 0){
		perror(NULL);
		return errno;
	}
	size_t shm_size = argc * 4096;
	if(ftruncate(shm_fd, shm_size) == -1){
		// aici functia scurteaza sau mareste obiectul de la shm_fd
		// la dimensiunea shm_size
		perror(NULL);
		shm_unlink(shm_name); // stergem obiectul de memorie
		return errno;
	}
	printf("Starting parent: %d\n",getpid());
	while(argv[i]){
		__pid_t pid = fork();
		if(pid < 0)
			return errno;
		else if (pid == 0){
			int *shm_ptr = mmap(0,4096,PROT_WRITE, MAP_SHARED, shm_fd,(i-1)*4096);
			// primul argument al functiei este adresa
			// la care sa fie incarcata in proces memoria alocata
			// apoi dimensiunea
			// urmeaza drepturile de acces (in cazul nostru de scrierer)
			// MAP_SHARED este un flag care indica faptul ca modificarile facute
			// de proces sa fie vizibile si in celelalte procese
			// descriptorul obtinul prin shm_open
			// si offset-ul
			collatz(atoi(argv[i]), shm_ptr, 0);
			printf("Done Parent %d Me %d\n",getppid(),getpid());
			munmap(shm_ptr,4096); //dezalocam memoria de la adresa indicata de pointerul shm_ptr
			return 0;
		}
		i++;
	}
	i = 1;
	while(argv[i]){
		wait();
		i++;
	}
	i = 1;
	while(argv[i]){
		printf("%d :",atoi(argv[i]));
		int *shm_ptr = mmap(0,4096, PROT_READ, MAP_SHARED, shm_fd, (i-1) * 4096);
		// aici avem drepturi de citire in procesul parinte
		int j = 0;
		while(shm_ptr[j]){
			printf("%d ",shm_ptr[j]);
			j++;
		}
		printf("\n");
		munmap(shm_ptr,4096);
		i++;
	}
	shm_unlink(shm_name);
	return 0;
}
