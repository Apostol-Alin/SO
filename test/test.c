#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <semaphore.h>

#define PROCESS_SHARED 1 /* pentru ca fiecare proces sa aiba acces la semafor */
#define TIMP_PENTRU_FREZAT 1
#define ADEVARAT 1
#define SEATS 5
#define BARBERS 3
#define CUSTOMERS 10

struct Monitor {
	sem_t barbers;
	sem_t customers;
	sem_t mutex;

	int freeSeats;
	pid_t customerPids[SEATS];
	pid_t barberPids[SEATS];
	int nextCustomer;
	int nextSeat;
};

int init_Monitor(struct Monitor *monitor) {
	// Cu semaforul penru barbers simulam o variabila conditionala
	// Daca un customer face post, atunci se trezeste un barber
	// Altfel, acesta doarme
	if (sem_init(&(monitor->barbers), PROCESS_SHARED, 0)) {
		perror ( NULL );
		return errno;
    	}
    	// Cu semaforul penru customers simulam o variabila conditionala
	// Daca un barber face post (adica acesta cheama un customer sa fie tuns), atunci se trezeste un customer
	// Altfel, acesta asteapta sa fie tuns pana in momentul in care il cheama un frizer
    	if (sem_init(&(monitor->customers), PROCESS_SHARED, 0)) {
		perror ( NULL );
		return errno;
    	}
    	// Simulam un mutex folosind cu semafor
    	if (sem_init(&(monitor->mutex), PROCESS_SHARED, 1)) {
		perror ( NULL );
		return errno;
    	}

    	monitor->freeSeats = SEATS;
    	monitor->nextCustomer = monitor->nextSeat = 0;

    	return 0;
}

void destroy_Monitor (struct Monitor *monitor) {
	sem_destroy(&(monitor->barbers));
	sem_destroy(&(monitor->customers));
	sem_destroy(&(monitor->mutex));
}


void barber(struct Monitor *monitor) {
    	int myNext;
    	pid_t customerPid;
    	
	printf("Barber %d joins shop\n", getpid());
	while(ADEVARAT){
	
		printf("Barber %d goes to sleep\n", getpid());
		struct timespec timeout;
    		clock_gettime(CLOCK_REALTIME, &timeout);
    		timeout.tv_sec += 4;
    		// Barber se va pune la coada (doarme, pana cand il trezeste un client)
    		// Daca ajunge sa stea peste program (nu mai sunt clienti), atunci procesul se incheie
		if (sem_timedwait(&(monitor->barbers), &timeout) == -1)
			return;
		sem_wait(&(monitor->mutex)); // Asteptam acces la mutex pentru a avea acces la variabilele din monitor
		myNext = monitor->nextCustomer; // Luam urmatorul Customer
		monitor->nextCustomer = ((monitor->nextCustomer) + 1) % SEATS; // Setam urmatorul Customer care sa fie servit
		customerPid = monitor->customerPids[myNext]; // Luam PID-ul Customer
		monitor->barberPids[myNext] = getpid(); // Punem PID-ul pentru Customer
		sem_post(&(monitor->mutex)); // Deblocam mutex-ul
		sem_post(&(monitor->customers)); // Chemam urmatorul Customer pentru frezat
		printf("Barber %d is serving the customer %d\n", getpid(), customerPid);
		sleep(TIMP_PENTRU_FREZAT);
		printf("Barber %d finished cutting the hair for customer %d\n", getpid(), customerPid);

	}
}

void customer(struct Monitor *monitor) {
    int mySeat;
    pid_t barberPid;
    sem_wait(&(monitor->mutex)); // Asteptam acces la mutex pentru a avea acces la variabilele din monitor
    printf("Customer %d enters shop\n", getpid());
    if((monitor->freeSeats) > 0){ // Verificam daca sunt locuri libere
        --(monitor->freeSeats); // Scadem numarul de locuri libere
        printf("Customer %d sits in waiting room.\n", getpid()); 
        mySeat = monitor->nextSeat; // Luam locul pe care trebuie sa ne asezam
        monitor->nextSeat = ((monitor->nextSeat) + 1) % SEATS; // Setam urmatorul loc liber
        monitor->customerPids[mySeat] = getpid(); // Punem PID-ul pentru Barber
        sem_post(&(monitor->mutex)); // Deblocam mutex-ul
        sem_post(&(monitor->barbers)); // Notificam un Barber ca trebuie sa serveasca un Customer
        sem_wait(&(monitor->customers)); // Asteptam ca un Barber sa ne cheme ca putem fi serviti
        sem_wait(&(monitor->mutex)); // Asteptam acces la mutex pentru a avea acces la variabilele din monitor
        barberPid = monitor->barberPids[mySeat]; // Luam PID-ul Barber
        (monitor->freeSeats)++; // Incrementam numarul de locuri de asteptare libere
        sem_post(&(monitor->mutex)); // Deblocam mutex-ul
        printf("Customer %d is having hair cut by barber %d.\n", getpid(), barberPid);
        
    }
    else{
        sem_post(&(monitor->mutex)); // Deblocam mutex-ul
        printf("Customer %d finds no seat and leaves.\n",getpid()); // Nu au fost gasite locuri libere
    
    }
   
}


int main () {

	char shm_name [] = "SleepingBarbers";
	int shm_fd;
	shm_fd = shm_open ( shm_name , O_CREAT | O_RDWR , S_IRUSR | S_IWUSR );
	if ( shm_fd < 0) {
		perror ( NULL );
		return errno ;
	}

	size_t shm_size = sizeof(struct Monitor) + (4096 - sizeof(struct Monitor)%4096);

	if ( ftruncate ( shm_fd , shm_size ) == -1) {
		perror ( NULL );
		shm_unlink ( shm_name );
		return errno ;
	}

	pid_t pids[BARBERS + CUSTOMERS + 3];

	struct Monitor *shm_ptr_init;
	shm_ptr_init = mmap (0 , sizeof(struct Monitor) , PROT_READ | PROT_WRITE , MAP_SHARED , shm_fd , 0 );
	if ( shm_ptr_init == MAP_FAILED ) {
		perror ( NULL );
		shm_unlink ( shm_name );
		return errno ;
	}

	if (init_Monitor(shm_ptr_init) != 0) {
    		perror(NULL);
    		return errno;
    	}

    	for (int i = 1; i <= BARBERS; i++){
    		pids[i] = fork();
    		if(pids[i] < 0)
    			return errno;
    		else
    			if(pids[i] == 0){
    				struct Monitor *shm_ptr;
				shm_ptr = mmap (0 , sizeof(struct Monitor) , PROT_READ | PROT_WRITE , MAP_SHARED , shm_fd , 0 );
				if ( shm_ptr == MAP_FAILED ) {
					perror ( NULL );
					shm_unlink ( shm_name );
					return errno ;
				}
				barber(shm_ptr);
				munmap(shm_ptr, sizeof(struct Monitor));

				exit(EXIT_SUCCESS);
    			}
    	}
    	
    	for (int i = 1; i <= CUSTOMERS; i++) {
		pids[BARBERS + i] = fork();

		if (pids[BARBERS + i] < 0)
			return errno;
		else
			if (pids[BARBERS + i] == 0) {

				struct Monitor *shm_ptr;
				shm_ptr = mmap (0 , sizeof(struct Monitor) , PROT_READ | PROT_WRITE , MAP_SHARED , shm_fd , 0 );
				if ( shm_ptr == MAP_FAILED ) {
					perror ( NULL );
					shm_unlink ( shm_name );
					return errno ;
				}
				customer(shm_ptr);
				munmap(shm_ptr, sizeof(struct Monitor));

				exit(EXIT_SUCCESS);
			}
	}

	for (int i = 1; i <= CUSTOMERS; i++)
		if (pids[BARBERS + i] != 0)
			wait(NULL);
	sleep(3);
	for (int i=1; i<=BARBERS; i++)
		if (pids[i] != 0)
			wait(NULL);

	destroy_Monitor(shm_ptr_init);
	munmap(shm_ptr_init, sizeof(struct Monitor));
	shm_unlink(shm_name);
	
	exit(EXIT_SUCCESS);
}
