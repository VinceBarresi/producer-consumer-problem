#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <synch.h>

#define MAX_THREAD_SLEEPTIME 2
#define BUFFER_SIZE 10
#define SEED 1000
int ret_val;
int totalSleepTime;
int noOfProducers = 0;
int noOfConsumers = 0;

static mutex_t monitor; //Static global variable's scope is inside this file and the value and memory location of static variable will remain same everytime it is called. 
int sleeptime;
typedef struct {
  int pc_buffer[BUFFER_SIZE];
  int in;
  int out;
  int g;
} bounded_buffer;
bounded_buffer Buffer1; //buffer shared between producer and consumer threads

void *Producer(void *param);
void *Consumer(void *param);

void *Producer(void *param) {
  
  int i, j, randomNumber, threadSleepTime;
  int sleeptime;
  srandom(SEED); 
  threadSleepTime = totalSleepTime;
  
  while(threadSleepTime > 0) { // loop until thread sleeps for the given input limit time   
    sleeptime = (int) random() % MAX_THREAD_SLEEPTIME; 
    threadSleepTime = threadSleepTime - sleeptime;  
    randomNumber = (int) random() % SEED;
    // caling mutex
    mutex_lock(&monitor);
    noOfProducers++;
    Buffer1.pc_buffer[Buffer1.in] = randomNumber;
    Buffer1.in = (Buffer1.in + 1) % BUFFER_SIZE;
    printf("Item %d added by Producer %d :  Buffer = [", randomNumber, param);
    
    for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
      printf("%d ,", Buffer1.pc_buffer[Buffer1.g]);
    }
    printf("%d] In = %d, Out = %d \n", Buffer1.pc_buffer[9], Buffer1.in, Buffer1.out);
    mutex_unlock(&monitor);
    //mutex unlock 
    srandom(randomNumber);
    sleep(sleeptime);
  } 
  pthread_exit(0);
}

void *Consumer(void *param) {
  
  int i, j, randomNumber, threadSleepTime, outValue, sleeptime;
  srandom(SEED); 
  threadSleepTime = totalSleepTime;
  
  while(threadSleepTime > 0) { 
    sleeptime = (int) random() % MAX_THREAD_SLEEPTIME;
    threadSleepTime = threadSleepTime - sleeptime;  
    randomNumber = (int) random() % SEED;
    mutex_lock(&monitor); {
      printf("Consumer Thread %d Waiting on lock\n", param);
    };
    
    if(Buffer1.out == Buffer1.in ) {
      printf("No Items left for Consumer %d :  Buffer = [", param);
      for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
        printf("%d,", Buffer1.pc_buffer[Buffer1.g]);
      }
      printf("%d] In = %d, Out=%d\n",Buffer1.pc_buffer[9],Buffer1.in,Buffer1.out);
    }
    else {
      noOfConsumers++;
      outValue = Buffer1.pc_buffer[Buffer1.out];
      Buffer1.pc_buffer[Buffer1.out] = 0;
      Buffer1.out = (Buffer1.out + 1) % BUFFER_SIZE;
      printf("Item %d taken by Consumer %d :  Buffer = [",outValue,param);
      for (Buffer1.g = 0; Buffer1.g < BUFFER_SIZE - 1; Buffer1.g = Buffer1.g + 1) {
        printf("%d ,",Buffer1.pc_buffer[Buffer1.g]);
      }
      printf("%d] In = %d, Out=%d\n",Buffer1.pc_buffer[9],Buffer1.in,Buffer1.out);
    } 
    mutex_unlock(&monitor); //release lock when finished with the critical section
    srandom(randomNumber);
    sleep(sleeptime);
  } 
  pthread_exit(0);
}

int main(int argc, char *argv[]) {
  
  pthread_t pid[atoi(argv[2])];
  pthread_t cid[atoi(argv[3])];
  pthread_attr_t attr;
  int count=0;
  
  if (argc != 4) {
    fprintf(stderr, "usage: <TotalThreadSleepTime> <NumberOfProducers> <NumberOfConsumers> \n");
    return -1;
  }
  
  if(atoi(argv[1]) < 0) {
    fprintf(stderr," %d must be >= 0 \n", atoi(argv[1]));
    return -1;
  }
  printf("Main \n");   
  Buffer1.in = 0;
  Buffer1.out = 0;
  totalSleepTime = atoi(argv[1]);
    
  printf("CLI Inputs -> TotalThreadSleepTime = %d NumberOfProducers = %d NumberOfConsumers = %d \n", totalSleepTime, atoi(argv[2]), atoi(argv[3]));
  mutex_init(&monitor, USYNC_THREAD, (void*) NULL);
  pthread_attr_init(&attr); //get the default attributes
   
  // create n producers
  for  (count=0; count < atoi(argv[2]); count++) {
    ret_val = pthread_create(&pid[count], &attr, Producer, (void *)count);
    printf("Created Producer Thread = %d count=%d\n" ,pid[count],(void *)count);
  }

  // create n consumers
  for  (count=0; count < atoi(argv[3]); count++) {
    ret_val = pthread_create(&cid[count], &attr, Consumer, (void *)count);
    printf("Created Consumer Thread = %d count=%d\n" ,cid[count],(void *)count);
  }

  /* join all producer threads, joining threads makes whole application wait until all producer-consumer threads finish their execution.
   otherwise it is possible that application may return the main function */
  for  (count=0; count < atoi(argv[2]); count++) {
    pthread_join(pid[count], NULL);
    printf("Join Producer thread = %d\n" ,count);
  }

  // join all consumer threads
  for  (count=0; count < atoi(argv[3]); count++) {
    pthread_join(cid[count], NULL);
    printf("Join Consumer thread = %d\n" ,count);
  }

  printf("Total items produced = %d\n", noOfProducers);
  printf("Total items consumed = %d\n", noOfConsumers);
  
  return 1;
}
