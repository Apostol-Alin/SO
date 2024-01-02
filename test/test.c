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

#define PROCESS_SHARED 1
#define TIMP_PENTRU_FREZAT 1
#define ADEVARAT 1
#define SEATS 5
#define BARBERS 3
#define CUSTOMERS 10

struct Monitor {
	sem_t barbers;
	sem_t customers;
	sem_t mutex;
	//sem_t counter_mutex;

	int freeSeats;
	pid_t customerPids[SEATS];
	pid_t barberPids[SEATS];
	int nextCustomer;
	int nextSeat;
	//int counter;
};

int init_Monitor(struct Monitor *monitor) {
	// Cu semaforul penru barbers simulam o variabila conditionala
	// Daca un customer face post, atunci se trezeste un barber
	// Altfel, acesta doarme
	if (sem_init(&(monitor->barbers), PROCESS_SHARED, 0)) {
		perror ( NULL );
		return errno;
    	}
    	if (sem_init(&(monitor->customers), PROCESS_SHARED, 0)) {
		perror ( NULL );
		return errno;
    	}
    	if (sem_init(&(monitor->mutex), PROCESS_SHARED, 1)) {
		perror ( NULL );
		return errno;
    	}
    	//if (sem_init(&(monitor->counter_mutex), PROCESS_SHARED, 1)) {
		//perror ( NULL );
		//return errno;
    	//}


	//monitor->counter = 1;
    	monitor->freeSeats = SEATS;
    	monitor->nextCustomer = monitor->nextSeat = 0;

    	return 0;
}

void destroy_Monitor (struct Monitor *monitor) {
	/*for(int i = 0; i < BARBERS; i++)
		sem_post(&(monitor->counter_mutex));
	for(int i = 0; i < CUSTOMERS; i++)
		sem_post(&(monitor->customers));
	for(int i = 0; i < BARBERS; i++)
		sem_post(&(monitor->barbers));
	for(int i = 0; i < BARBERS; i++)
		sem_post(&(monitor->mutex));*/
	sem_destroy(&(monitor->barbers));
	sem_destroy(&(monitor->customers));
	sem_destroy(&(monitor->mutex));
	//sem_destroy(&(monitor->counter_mutex));
}


void barber(struct Monitor *monitor) {
    	int myNext;
    	pid_t customerPid;
    	
	printf("Barber %d joins shop\n", getpid());
	while(ADEVARAT){
		printf("Barber %d goes to sleep\n", getpid());
		/*sem_wait(&(monitor->counter_mutex));
		if(monitor->counter >= CUSTOMERS){
			sem_post(&(monitor->counter_mutex));
			break;
		}
		else
			sem_post(&(monitor->counter_mutex));*/
		sem_wait(&(monitor->barbers)); // Barber goes to barber queue
		sem_wait(&(monitor->mutex)); // Lock the mutex to secure the seats
		myNext = monitor->nextCustomer;
		monitor->nextCustomer = ((monitor->nextCustomer) + 1) % SEATS;
		customerPid = monitor->customerPids[myNext]; // Get the Customer's PID
		monitor->barberPids[myNext] = getpid(); // Give my PID to the Customer
		sem_post(&(monitor->mutex)); // Unlock the mutex to leave access to seats
		sem_post(&(monitor->customers)); // Call the next costumer for his freza
		printf("Barber %d is serving the customer %d\n", getpid(), customerPid);
		sleep(TIMP_PENTRU_FREZAT);
		printf("Barber %d finished cutting the hair for customer %d\n", getpid(), customerPid);
		/*sem_wait(&(monitor->counter_mutex));
		monitor->counter = monitor->counter + 1;
		if(monitor->counter >= CUSTOMERS){
			sem_post(&(monitor->counter_mutex));
			break;
		}
		else
			sem_post(&(monitor->counter_mutex));*/

	}
}

void customer(struct Monitor *monitor) {
    int mySeat;
    pid_t barberPid;
    sem_wait(&(monitor->mutex));
    printf("Customer %d enters shop\n", getpid());
    if((monitor->freeSeats) > 0){
        --(monitor->freeSeats);
        printf("Customer %d sits in waiting room.\n", getpid());
        mySeat = monitor->nextSeat;
        monitor->nextSeat = ((monitor->nextSeat) + 1) % SEATS;
        monitor->customerPids[mySeat] = getpid();
        sem_post(&(monitor->mutex));
        sem_post(&(monitor->barbers));
        sem_wait(&(monitor->customers));
        sem_wait(&(monitor->mutex));
        barberPid = monitor->barberPids[mySeat];
        (monitor->freeSeats)++;
        sem_post(&(monitor->mutex));
        printf("Customer %d is having hair cut by barber %d.\n", getpid(), barberPid);
        
    }
    else{
        sem_post(&(monitor->mutex));
        /*sem_wait(&(monitor->counter_mutex));
   	monitor->counter = monitor->counter + 1;
   	sem_post(&(monitor->counter_mutex));*/
        printf("Customer %d finds no seat and leaves.\n",getpid());
    
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
			
	for (int i = 1; i <= BARBERS; i++)
		if (pids[i] != 0){
			kill(pids[i], SIGTERM);
			waitpid(pids[i], NULL, 0); 
		}

	destroy_Monitor(shm_ptr_init);
	munmap(shm_ptr_init, sizeof(struct Monitor));
	shm_unlink(shm_name);

	exit(EXIT_SUCCESS);
	//return 0;
}
