#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "hw2_syscalls.h"

int main() {
  int child_pid1, child_pid2;
  int a = 0, b = 1, c;
  struct sched_param param = { 100 };
  sched_setscheduler(getpid(), SCHED_SHORT, &param);

  printf("// clock = %d\n", time(NULL));
  getchar();

  child_pid1 = fork();
  if (child_pid1 == 0) {
    printf("// %d 6) This is the first child! (PID=%d)\n", time(NULL), getpid());
    fflush(stdout);
    
    param.sched_priority = 10000;
    sched_setscheduler(getpid(), SCHED_SHORT, &param);
    
    while (short_query_remaining_time(getpid()) > 5000) {
      c = a + b;
      a = b;
      b = c;
    }
  } else {
    // This is the parent.
    child_pid2 = fork();
    if (child_pid2 == 0) {
      printf("// %d 3) This is the second child! (PID=%d)\n", time(NULL), getpid());
      fflush(stdout);
      param.sched_priority = 9900;
      sched_setscheduler(getpid(), SCHED_SHORT, &param);

      while (short_query_remaining_time(getpid()) > 5000) {
        c = a + b;
        a = b;
        b = c;
      }
    } else {
      printf("// %d 1) This is the parent.\n", time(NULL));
      fflush(stdout);
      printf("// %d 2) Waiting for first child ...\n", time(NULL));
      fflush(stdout);
      printf("// %d 4) %d Done.\n", time(NULL), wait());
      fflush(stdout);
      printf("// %d 5) Waiting for second child ...\n", time(NULL));
      fflush(stdout);
      printf("// %d 7) %d Done.\n", time(NULL), wait());
      fflush(stdout);
    }
  }
  
  return 0;
}
