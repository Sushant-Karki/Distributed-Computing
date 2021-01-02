#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "header.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

int factors[10];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

unsigned long int rotate(struct memory arg,unsigned long int num){
  arg.number = num;
  return (arg.number >> 1)|(arg.number << (32 - 1));
}

void *factorize(void *args) {
   struct memory *arg = (struct memory *)args;
   int flag = 0;
   unsigned long int num = (unsigned)arg->number;
   int parent = (int)arg->pid;
   int slot_id_local = (int)arg->slot_id;
   ((struct memory *)args)->slot[slot_id_local] = parent;
   ((struct memory *)args)->slot_status[slot_id_local] += 1;
   int thread = (int)arg->tid;
   printf("Thread ID: %d \t Parent ID: %d \t num is %lu\n", thread, parent ,num);
   arg->token = true;
   //do stuff here
   for(int i = 1;i <= (unsigned long int)sqrt(num); i++){
     if(num%i==0){
       pthread_mutex_lock(&lock);
         sprintf( ((struct memory *)args)->output,"Request ID : %d,\t Number : %lu,\t factor : %lu * %lu \n",parent,num,i,(unsigned long int)(num/(unsigned long int)i));
         //let the client know that output is ready to be printed.
         ((struct memory *)args)->output_token = true;
         while(((struct memory *)args)->output_token == true){;
         }// waits until client thread has printed the output
       pthread_mutex_unlock(&lock);
       flag += 1;
     }
   }
   pthread_mutex_lock(&lock);
   factors[slot_id_local] += flag*2;
   pthread_mutex_unlock(&lock);
   ((struct memory *)args)->slot_status[slot_id_local] -= 1;
   if(((struct memory *)args)->slot_status[slot_id_local] == 0){
     printf("\nRequest no : %d is complete\n",parent);
     pthread_mutex_lock(&lock);
      ((struct memory *)args)->end = clock();
      double time_spent = (double)(((struct memory *)args)->end - ((struct memory *)args)->begin)/CLOCKS_PER_SEC;
      sprintf( ((struct memory *)args)->output,"\nRequest no : %d is complete\t time taken was %f\n",parent,time_spent);
      ((struct memory *)args)->output_token = true;
      while(((struct memory *)args)->output_token == true){;
      }// waits until client thread has printed the output
    pthread_mutex_unlock(&lock);
   }
   //printf("\nslot is now : %d \n",((struct memory *)args)->slot_status[parent]);
   pthread_exit(NULL);
}


void main(void){
  key_t         ShmKEY;
  int           ShmID;
  struct memory *ShmPTR;

  ShmKEY = ftok(".",'x');
  ShmID = shmget(ShmKEY, sizeof(struct memory),0666);
  if (ShmID<0){
    printf("*** shmget error (server) ***\n");
    exit(1);
  }
  printf("  server has received a shared memory\n");

  ShmPTR = (struct memory *) shmat(ShmID, NULL, 0);
  if((int) ShmPTR == -1){
    printf("shmat error (server)\n");
    exit(1);
  }
  printf("server has attached the shared memory\n");

  memset(factors, 0, 10);
  memset(ShmPTR->slot_status, 0, 10);
  ShmPTR->slot_id = -1;
//
  while(1){
      while(ShmPTR->clientflag == 0){;
      }
      if(ShmPTR->q == true){
        break;
      }

      if(ShmPTR->pid > 9){
        int i = 0;
            while(1){
              if(i>9){
                i = 0;
              }
              if(ShmPTR->slot_status[i] == 0){
                ShmPTR->slot_id = i;
                factors[i] = 0;
                break;
              }
              i += 1;
            }
      }
      else{
        ShmPTR->slot_id += 1;
      }

      printf("\nThe process has started\n");
      printf("Number recieved from client was : %d\n", ShmPTR->number);
      int n_threads = 32;
      pthread_t threads[n_threads];
      for( int i = 0; i < n_threads; i++ ) {
         ShmPTR->tid = i;
         pthread_create(&threads[i], NULL, factorize, (void *)ShmPTR);
         ShmPTR->token = false;
         while(ShmPTR->token == false){;
         }
         ShmPTR->number = rotate(*ShmPTR, ShmPTR->number);
      }
      // client may send another request.
      ShmPTR->clientflag = 0;
  }
//

  // wait till all slot is complete
  while(1){
    int sum = 0;
    for(int i = 0; i < 10; i++){
      sum += ShmPTR->slot_status[i];
    }
    // change 20 to 10*n_threads
    if(sum == 0){
      break;
    }
  }

  // print the slot for results.
  printf("\n\n");
  printf("Slot_status   : \t");
  for(int i = 0; i < 10; i++){
    printf("%d\t", ShmPTR->slot_status[i]);
  }
  printf("\n");
  printf("Request ID    : \t");
  for(int i = 0; i < 10; i++){
    printf("%d\t", ShmPTR->slot[i]);
  }
  printf("\n");
  printf("Factors       : \t");
  for(int i = 0; i < 10; i++){
    printf("%d\t", factors[i]);
  }
  printf("\n");


  shmdt((void*)ShmPTR);
  printf("server has detached its memory\n");
  pthread_exit(NULL);
  printf("  server exits\n");
  exit(0);
}
