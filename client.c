#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include "header.h"
#define BUF_SIZE 1024


void *printing(void *args) {
  while(1){
    while(((struct memory *)args)->output_token == false){;
    }
    printf(((struct memory *)args)->output);
    ((struct memory *)args)->output_token = false;
    if(((struct memory *)args)->q == true){
      break;
    }
  }
  pthread_exit(NULL);
}


int main(int argc, char * argv[]) {

    char          client_buf[BUF_SIZE];
    char          quit;
    key_t         ShmKEY;
    int           ShmID;
    struct memory *ShmPTR;
    ShmKEY = ftok(".",'x');

    //shared memory created
    ShmID = shmget(ShmKEY, sizeof(struct memory), IPC_CREAT | 0666);
    if(ShmID < 0){
      printf("*** shmget error (client) ***\n");
      exit(1);
    }
    printf("client has recieved a shared memory \n");

    // pointer to the shared memory ShmKEY
    ShmPTR = (struct memory*) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
        printf("*** shmat error (client) ***\n");
        exit(1);
    }
    printf("client has attached the shared memory ...\n");

    ShmPTR->clientflag = 0;
    ShmPTR->pid = -1;
    ShmPTR->output_token = false;

    //A thread is created to output the factors.
    pthread_t thread;
    pthread_create(&thread, NULL, printing, (void *)ShmPTR);

    //Communicating with server through shared memory
    while(1){

        while(ShmPTR->clientflag != 0){;
        // waiting for number to be read by thread.
        }
        ShmPTR->pid += 1;

        printf("> ");
        // read from stdin to client_buf
        scanf("%s",client_buf);

        if(strcmp(client_buf,"quit")==0){
            ShmPTR->q = true;
        }
        if(ShmPTR->q == true){
          ShmPTR->clientflag = 1;
          break;
        }
        else{
            ShmPTR->begin = clock();
            ShmPTR->number = (unsigned)atoi(client_buf);
            //printf("number sent from client was : %d\n",atoi(client_buf));
            ShmPTR->clientflag = 1;
            //printf("client has filled the shared memory...\n");
        }

        //printf("client has detected the completion of its server\n");
    }
    shmdt((void*) ShmPTR);
    printf("Client has dettached its shared memory\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("client has removed its shared memory\n");
    //pthread_exit(NULL);
    printf("client exits\n");
    exit(0);
}
