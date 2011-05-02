#include <string>

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "test.h"
#include "hw2_syscalls.h"

/*
 * This test checks that the parent runs before it's two children, and that the
 * second child runs before the first.
 * It does this by setting itself as a short process, then creating two
 * children, and setting their requested time to be higher. Since the
 * differences are less than 10 jiffies at first, the parent should be able to
 * run and eventually the second child should be able to run before the first.
 */
bool testTwoChildren_SecondChildBeforeFirstChild() {
  int child_pid1, child_pid2;
  int a = 0, b = 1, c;
  struct sched_param param = { 100 };
  sched_setscheduler(getpid(), SCHED_SHORT, &param);

  child_pid1 = fork();
  if (child_pid1 == 0) {
    // This is the first child.
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
      // This is the second child.
      param.sched_priority = 9900;
      sched_setscheduler(getpid(), SCHED_SHORT, &param);

      while (short_query_remaining_time(getpid()) > 5000) {
        c = a + b;
        a = b;
        b = c;
      }
    } else {
      // This is the parent.
      ASSERT_TRUE(wait() == child_pid2);
      ASSERT_TRUE(wait() == child_pid1);
    }
  }
  
  return true;
}

int main() {
	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testTwoChildren_SecondChildBeforeFirstChild);

  return 0;
}
