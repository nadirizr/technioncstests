#include <string>

#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#include "test.h"
#include "hw2_syscalls.h"

std::string location;

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
  ASSERT_TRUE(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);

  sleep(1);
  
  child_pid1 = fork();
  if (child_pid1 == 0) {
    // This is the first child.
    param.sched_priority = 10000;
    ASSERT_TRUE(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);
    
    while (short_query_remaining_time(getpid()) > 7000) {
      c = a + b;
      a = b;
      b = c;
    }
    
    exit(0);
  } else {
    // This is the parent.
    child_pid2 = fork();
    if (child_pid2 == 0) {
      // This is the second child.
      param.sched_priority = 9900;
      ASSERT_TRUE(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);

      while (short_query_remaining_time(getpid()) > 7000) {
        c = a + b;
        a = b;
        b = c;
      }

      exit(0);
    } else {
      // This is the parent.
      ASSERT_TRUE((int)wait(NULL) == child_pid2);
      ASSERT_TRUE((int)wait(NULL) == child_pid1);
    }
  }
  
  return true;
}

/*
 * This test checks that the parent runs before it's child, if the difference
 * in remaining time is smaller than 10 jiffies.
 * It does this by setting itself as a short process with enough time, then
 * creating a child, which should get less time to run.
 */
bool testOneChild_FatherBeforeChildLessThan10Jiffies() {
  int child_pid;
  int a = 0, b = 1, c;
  struct sched_param param = { 200 };
  ASSERT_TRUE(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);

  sleep(1);
  
  child_pid = fork();
  if (child_pid == 0) {
    // This is the child.
    while (short_query_remaining_time(getpid()) > 0) {
      c = a + b;
      a = b;
      b = c;
    }
    
    exit(0);
  } else {
    // This is the parent.
    
    while (short_query_remaining_time(getpid()) > 10) {
      c = a + b;
      a = b;
      b = c;
    }
    
    ASSERT_TRUE(waitpid(child_pid, NULL, WNOHANG) == 0);
    
    ASSERT_TRUE(wait(NULL) == child_pid);
  }
  
  return true;
}
/*
 * This test checks that the child runs before it's parent, if the difference
 * in remaining time is larger than 10 jiffies.
 * It does this by setting itself as a short process with enough time, then
 * creating a child, which should get more time to run.
 */
bool testOneChild_ChildBeforeFatherMoreThan10Jiffies() {
  int child_pid;
  int a = 0, b = 1, c;
  struct sched_param param = { 10000 };
  ASSERT_TRUE(sched_setscheduler(getpid(), SCHED_SHORT, &param) == 0);

  sleep(1);

  child_pid = fork();
  if (child_pid == 0) {
    // This is the child.
    while (short_query_remaining_time(getpid()) > 3000) {
      c = a + b;
      a = b;
      b = c;
    }
    
    exit(0);
  } else {
    // This is the parent.
    ASSERT_TRUE(waitpid(child_pid, NULL, WNOHANG) == child_pid);
  }
  
  return true;
}

int main() {
	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testTwoChildren_SecondChildBeforeFirstChild);
  RUN_TEST(testOneChild_FatherBeforeChildLessThan10Jiffies);
  RUN_TEST(testOneChild_ChildBeforeFatherMoreThan10Jiffies);

  return 0;
}
