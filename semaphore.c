#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <synch.h>
#include <semaphore.h>

#define MAX_THREAD_SLEEPTIME 2
#define BUFFER_SIZE 10
#define SEED 1000
int ret_val;
int totalSleepTime;
int sleepTime;
int noOfProducers = 0;
int noOfConsumers = 0;

typedef struct {
  int pc_buffer[BUFFER_SIZE];
  int in;
  int out;
  int g;
} bounded_buffer;
bounded_buffer Buffer1; //buffer shared between producer and consumer threads

void *Producer(void *param);
void *Consumer(void *param);
sem_t producer_sem;
sem_t consumer_sem;
sem_t mutex;

void *Producer(void *param) {
  int i, j, randomNumber, threadSleepTime, sleeptime;
  srandom(SEED); 
  threadSleepTime = totSleepTime;
  while(threadSleepTime > 0) { // Loop until thread sleeps for the given input limit time 
    
    sleeptime = (int) random() % MAX_THREAD_SLEEPTIME; 
    printf("Thread No %3d Sleeping for = %d Remaining sleeptime = %d \n", param, sleeptime, threadSleepTime);
    threadSleepTime = threadSleepTime - sleeptime;  
    randomNumber = (int) random() % SEED;
    
    sem_wait(&consumer_sem);
    sem_wait(&mutex); 
    
    noOfProducers++;
    Buffer1.pc_buffer[Buffer1.in] = randomNumber;
    Buffer1.in = (Buffer1.in + 1) % BUFFER_SIZE;
    printf(" Buffer item %d added by Producer %d :  Buffer = [", randomNumber, param);
    for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g += 1) {
      
      printf("%d,", Buffer1.pc_buffer[Buffer1.g]);
    }
    printf("%d] In = %d, Out=%d\n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
    sem_post(&mutex);
    sem_post(&producer_sem);
    srandom(randomNumber);
    sleep(sleeptime);
  } 
  pthread_exit(0);
}

void *Consumer(void *param) {
  
  int i, j, randomNumber, threadSleepTime, outValue, sleepTime;
  srandom(SEED); 
  threadSleepTime = totSleepTime;
  
  while(threadSleepTime > 0){ 
    
    sleeptime = (int) random() % MAX_THREAD_SLEEPTIME;
    printf("Thread No %3d Sleeping for = %d Remaining sleeptime = %d \n", param, sleeptime, threadSleepTime);
    threadSleepTime = threadSleepTime - sleeptime;  
    randomNumber = (int) random() % SEED;
      ]
    sem_wait(&producer_sem);
    sem_wait(&mutex);

    if(Buffer1.out == Buffer1.in) {
    
      for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g += 1) {
          
          printf("%d ,", Buffer1.pc_buffer[Buffer1.g]);
        }
      printf("%d] In = %d, Out=%d \n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
      return 1;
    }
    else {
      
      noOfConsumers++;
      outValue = Buffer1.pc_buffer[Buffer1.out];
      Buffer1.pc_buffer[Buffer1.out] = 0;
      Buffer1.out = (Buffer1.out + 1) % BUFFER_SIZE;
      printf("Buffer item %d taken by Consumer %d :  Buffer = [", outValue, param);
      for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g += 1) {
        
        printf("%d ,", Buffer1.pc_buffer[Buffer1.g]);
      }
      printf("%d] In = %d, Out=%d \n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
    }
    sem_post(&mutex);
    sem_post(&consumer_sem);
  
    srandom(randomNumber);
    sleep(sleeptime);
  } 
  pthread_exit(0);
}

int main(int argc, char *argv[]) {
  
  pthread_t producerID[atoi(argv[2])];
  pthread_t consumerID[atoi(argv[3])];
  pthread_attr_t attr;
  int count = 0;
  if (argc != 4) {
    
    fprintf(stderr, "usage: <TotalThreadSleepTime> <NumberOfProducers> <NumberOfConsumers> \n");
    return -1;
  }
  
  if (atoi(argv[1]) < 0) {
    
    fprintf(stderr, "%d must be>=0 \n", atoi(argv[1]));
    return -1;
  }
  // initializing producer semaphore
  sem_init(&producer_sem, 0, 0);
  
  // initializing consumer semaphore
  sem_init(&consumer_sem, 0, BUFFER_SIZE);
  
  // initiaizing mutex
  pthread_attr_init(&attr);
  
  //get the default attributes
  sem_init(&mutex,0,1);
   
  printf("Main \n");    
  Buffer1.in = 0;
  Buffer1.out = 0;
  totSleepTime = atoi(argv[1]);
    
  printf("CLI Inputs -> TotalThreadSleepTime= %d NumberOfProducers= %d NumberOfConsumers= %d \n", totSleepTime, atoi(argv[2]), atoi(argv[3]));

  // Create n producers
  for (count = 0; count < atoi(argv[2]); count++) {
    
    ret_val = pthread_create(&producerID[count], &attr, Producer, (void *) count);
    printf("Created Producer Thread = %d count = %d \n", producerID[count], (void *) count);
  }

  // Create n consumers
  for (count = 0; count < atoi(argv[3]); count++) {
    
    ret_val = pthread_create(&consumerID[count], &attr, Consumer, (void *)count);
    printf("Created Consumer Thread = %d count = %d \n", consumerID[count], (void *)count);
  }

  // join all of the producer threads
  for (count=0; count < atoi(argv[2]); count++) {
    
    pthread_join(producerID[count], NULL);
    printf("join Producer thread = %d \n", count);
  }

  // join all of the consumer threads
  for (count = 0; count < atoi(argv[3]); count++) {
    
    pthread_join(consumerID[count], NULL);
    printf("join Consumer thread = %d \n", count);
  }

  printf("Total items produced = %d \n", noOfProducers);
  printf("Total items consumed = %d \n", noOfConsumers);
  
  return 1;
}
