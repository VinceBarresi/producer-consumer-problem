#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <synch.h>

#define LOCKED 1
#define MAX_THREAD_SLEEPTIME 2
#define BUFFER_SIZE 10
#define SEED 1000
int ret_val;
int totalSleepTime;
volatile int last_value;
volatile int lock = 0;
int noOfProducer = 0;
int noOfConsumer = 0;
int sleeptime;
typedef struct {
	int pc_buffer[BUFFER_SIZE];
	int in;
	int out;
	int g;
} bounded_buffer;
bounded_buffer Buffer1; // buffer shared between producer and consumer threads

void *Producer(void *param);
void *Consumer(void *param);
int test_and_set(int *x) ;

int test_and_set(int *x) {
	last_value = *x;
	*x = 1;
	return last_value;
}


void *Producer(void *param) {
	int i, j, randomNumber, threadSleepTime;
	int sleeptime;
	srandom(SEED); 
	threadSleepTime = totalSleepTime;
	while(threadSleepTime > 0) { // Loop until thread sleeps for the given input limit time	
		sleeptime = (int) random() % MAX_THREAD_SLEEPTIME; 
		printf("Thread No %3d Sleeping for = %d Remaining sleeptime = %d\n", param, sleeptime, threadSleepTime);
		threadSleepTime = threadSleepTime - sleeptime;  
		randomNumber = (int) random() % SEED;
		while (test_and_set(&lock) == 1) { 
			printf("Producer Thread %d Waiting on lock\n", param); 
		};
		noOfProducer++;
		Buffer1.pc_buffer[Buffer1.in] = randomNumber;
		Buffer1.in = (Buffer1.in + 1) % BUFFER_SIZE;
		printf("Item %d added by Producer %d :  Buffer = [", randomNumber, param);
		for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
			printf("%d ,",Buffer1.pc_buffer[Buffer1.g]);
		}
		printf("%d] In = %d, Out=%d\n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
		lock = 0; //release lock when finished with the critical section
		srandom(randomNumber);
		sleep(sleeptime);
	}	
	pthread_exit(0);
}

void *Consumer(void *param) {
	int i,j,randomNumber,threadSleepTime,outValue;
	int  sleeptime;
	srandom(SEED); 
	threadSleepTime = totalSleepTime;
	while(threadSleepTime>0){	
		sleeptime = (int) random() % MAX_THREAD_SLEEPTIME;
		//printf("Thread No %3d Sleeping for=%d Remaining sleeptime=%d\n",param,sleeptime,threadSleepTime);
		threadSleepTime = threadSleepTime - sleeptime;  
		randomNumber = (int) random() % SEED;
		while (test_and_set(&lock) == 1) {
			printf("Consumer Thread %d Waiting on lock\n",param);
		};
		if(Buffer1.out == Buffer1.in ) {
			printf("No Items left for Consumer %d :  Buffer = [", param);
			for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
				printf("%d ,",Buffer1.pc_buffer[Buffer1.g]);
			}
			printf("%d] In = %d, Out=%d\n",Buffer1.pc_buffer[9],Buffer1.in,Buffer1.out);
		}
		else {
			noOfConsumer++;
			outValue = Buffer1.pc_buffer[Buffer1.out];
			Buffer1.pc_buffer[Buffer1.out]=0;
			Buffer1.out = (Buffer1.out + 1) % BUFFER_SIZE;
			printf("Item %d taken by Consumer %d :  Buffer = [",outValue,param);
			for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
				printf("%d ,", Buffer1.pc_buffer[Buffer1.g]);
			}
			printf("%d] In = %d, Out = %d\n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
    		}	
		lock = 0; //release lock when finished with the critical section
		srandom(randomNumber);
		sleep(sleeptime);
	}	
	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	pthread_t pid[atoi(argv[2])];
	pthread_t cid[atoi(argv[3])];
	pthread_attr_t attr;
    int counter = 0;
    if (argc != 4) {
		fprintf(stderr, "usage: <TotalThreadSleepTime> <NumberOfProducers> <NumberOfConsumers> \n");
		return -1;
	}
	if(atoi(argv[1]) < 0) {
		fprintf(stderr, "%d must be >= 0\n", atoi(argv[1]) );
		return -1;
	}
    printf("Main Program\n");		
	Buffer1.in = 0;
	Buffer1.out = 0;
	totalSleepTime = atoi(argv[1]);
	pthread_attr_init(&attr); //get the default attributes
	printf("CLI Inputs -> TotalThreadSleepTime = %d NumberOfProducers = %d NumberOfConsumers = %d \n", totalSleepTime, atoi(argv[2]), atoi(argv[3]));

	// create n producers
	for  (counter = 0; counter < atoi(argv[2]); counter++) {
		ret_val = pthread_create(&pid[counter], &attr, Producer, (void *)counter);
		printf("Created Producer Thread = %d counter = %d\n", pid[counter], (void *)counter);
	}

	// create n consumers
	for  (counter=0; counter < atoi(argv[3]); counter++) {
		ret_val = pthread_create(&cid[counter], &attr, Consumer, (void *)counter);
		printf("Created Consumer Thread = %d counter = %d\n", cid[counter], (void *)counter);
	}

	// join all producer threads
	for  (counter=0; counter < atoi(argv[2]); counter++) {
		pthread_join(pid[counter], NULL);
		printf("Join Producer thread = %d\n", counter);
	}

	// join all consumer threads
	for  (counter=0; counter < atoi(argv[3]); counter++) {
		pthread_join(cid[counter], NULL);
		printf("Join Consumer thread = %d\n", counter);
	}

	printf("Total items produced = %d\n", noOfProducer);
	printf("Total items consumed = %d\n", noOfConsumer);
	
	return 1;
}
