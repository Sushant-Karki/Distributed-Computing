#define NUM 10;
int counter = 0;

void *printer(void *arg)
{
  for (int i=0;i<NUM;i++){
    printf("count = %d\n",counter);
    sleep(1);
  }
  return NULL;
}

int main(){
  // creating theread
  pthread_t tid;
  // thread created for the function printer
  pthread_create(&tid,NULL,printer,NULL);
  for (int i=0; i<NUM; i++){
    counter++;
    sleep(1);
  }
  pthread_join(tid, NULL);
  return 0;

}
