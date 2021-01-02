#define NOT_READY -1
#define FILLED     0
#define TAKEN      1
#include <stdbool.h>
#include <time.h>

struct memory {
  int clientflag;
  unsigned long int number;
  int slot_status[10];
  int slot[10];
  char output[250];
  int tid;
  int pid;
  bool token;
  bool output_token;
  bool q;
  int slot_id;
  clock_t begin;
  clock_t end;
  int time_seconds;

} ;
