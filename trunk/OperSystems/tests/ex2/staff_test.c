#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#include "hw2_syscalls.h"

/* 5 milliseconds time difference. */
#define TIME_ERROR 0.005

int times_equal(double time1, double time2) {
  if (time1 > time2) {
    return ((time1 - time2) < TIME_ERROR);
  } else {
    return ((time2 - time1) < TIME_ERROR);
  }
}

double get_current_time() {
  struct timeb tb;
  ftime(&tb);
  return ((double)tb.time) + (((double)tb.millitm) / 1000.0);
}

double get_current_clock() {
  double time = (double)(clock() * 1000 / CLOCKS_PER_SEC);
  return time / 1000.0;
}

int test2() {
  int error = 0;
  
  int a = 1, b = 1;
  int pid;
  double start_time;
  double current_time;
  struct sched_param params = { 2000 };
  
  printf("[Test 2]: Starting Run:\n");
  
  if (pid = fork()) {
    /* This is the father: */

    /* Measure the time. */
    start_time = get_current_time();

    /* Set child to be short - it should now start running. */
    sched_setscheduler(pid, 4, &params);

    /* Child is by now overdue - check if the time that passed is 2 seconds. */
    current_time = get_current_time() - start_time;
    if (!times_equal(current_time, 2.0)) {
      printf("[Test 2]: ERROR: time is %f, should have been 2!\n", current_time);
      error = 1;
    }

    /* Sleep 1 second to allow the child to run as overdue. */
    sleep(1);

    /* Verify that the son's overdue time is positive. */
    if (short_query_overdue_time(pid) <= 0) {
      printf("[Test 2]: ERROR: overdue time should be more than 0!\n");
      error = 1;
    };

    if (!error) {
      printf("[Test 2]: PASSED!\n");
    }
  } else {
    /* This is the child - it should just run for 4 seconds. */
    start_time = get_current_time();
    while (get_current_time() - start_time < 4.0) {
      a = a + b;
      b = a;
    }
  }

  return 0;
}

int test5() {
  struct sched_param params;
  int dead_child_pid;

  dead_child_pid = fork();
  if (dead_child_pid == 0) {
    return 0;
  }
  waitpid(dead_child_pid, NULL, 0);

  params.sched_priority = 100;
  printf("[Test 5]: Setting dead PID param to be 100: %d ",
      sched_setparam(dead_child_pid, &params));
  printf("(errno = %d)\n", errno);

  params.sched_priority = 30001;
  printf("[Test 5]: Setting our PID param to be 30001: %d ",
      sched_setparam(getpid(), &params));
  printf("(errno = %d)\n", errno);

  params.sched_priority = -1;
  printf("[Test 5]: Setting our PID param to be -1: %d ",
      sched_setparam(getpid(), &params));
  printf("(errno = %d)\n", errno);
  
  printf("[Test 5]: Checking policy for our PID: %d\n",
      sched_getscheduler(getpid()));
  
  params.sched_priority = 1000;
  sched_setscheduler(getpid(), 4, &params);
  
  params.sched_priority = 500;
  printf("[Test 5]: Reducing requested time from 1000 to 500: %d ",
      sched_setparam(getpid(), &params));
  printf("(errno = %d)\n", errno);
  
  params.sched_priority = 15000;
  printf("[Test 5]: Increasing requested time from 1000 to 15000: %d\n",
      sched_setparam(getpid(), &params));

  params.sched_priority = 30000;
  printf("[Test 5]: Increasing requested time from 15000 to 30000: %d\n",
      sched_setparam(getpid(), &params));

  return 0;
}

int test7() {
  int error = 0;
  
  int a = 1, b = 1;
  int i;
  int pid;
  double start_time;
  double current_time;
  struct sched_param params = { 10000 };
  
  printf("[Test 7]: Starting Run:\n");
  
  if (pid = fork()) {
    /* This is the father: */

    /* Do this 3 times each time increasing by 3 seconds. */
    for (i = 1; i <= 3; ++i) {
      /* Measure the time. */
      start_time = get_current_time();

      /* Set child to be short - it should now start running. */
      params.sched_priority = 10000 * i;
      sched_setscheduler(pid, 4, &params);

      /* Child is by now overdue - check if the time that passed is 10 seconds. */
      current_time = get_current_time();
      if (!times_equal(current_time - start_time, 10)) {
        printf("[Test 7]: ERROR: time is %f, should have been 10 (iteration %d)!\n",
               current_time - start_time, i);
        error = 1;
      }

      /* Child remaining time should be 0. */
      if (short_query_remaining_time(pid) != 0) {
        printf("[Test 7]: ERROR: child remaining time is %d, should have been 0 (iteration %d)!\n",
               short_query_overdue_time(pid), i);
        error = 1;
      }

      /* Child overdue time should be close to 0. */
      if (short_query_overdue_time(pid) > 5 ||
          short_query_overdue_time(pid) < 0) {
        printf("[Test 7]: ERROR: child overdue time is %d, should be close to 0 (iteration %d)!\n",
               short_query_overdue_time(pid), i);
        error = 1;
      }
    }

    if (!error) {
      printf("[Test 7]: PASSED!\n");
    }
  } else {
    /* This is the child - it should just run to infinity. */
    start_time = get_current_time();
    while (get_current_time() - start_time < 35.0) {
      a = a + b;
      b = a;
    }
  }

  return 0;
}

int test8() {
  int error = 0;
  
  int a = 1, b = 1;
  int i;
  int father_pid, child_pid;
  int father_status, child_status;
  double start_time;
  double current_time;
  struct sched_param params = { 10000 };
  
  printf("[Test 8]: Starting Run:\n");
  fflush(stdout);
  
  if (father_pid = fork()) {
    /* This is the main: */

    /* Wait for the father to finish check if it returned an error. */
    waitpid(father_pid, &father_status, 0);

    if (!father_status) {
      printf("[Test 8]: PASSED!\n");
    }
  } else {
    /* This is the father: */
    
    /* Father sets itself to be short with 10 seconds. */
    params.sched_priority = 10000;
    sched_setscheduler(getpid(), 4, &params);
    
    /* Measure the start time. */
    start_time = get_current_time();

    if (child_pid = fork()) {
      /* This is the father: */

      /* Son ran first - current time should be around 4900 milliseconds. */
      current_time = get_current_time();
      if (!times_equal(current_time - start_time, 4.9)) {
        printf("[Test 8]: ERROR: father time check is %f, should be around 4.9!\n",
               current_time - start_time);
        error = 1;
      }
      
      /* Verify that we have around 5100 milliseconds of run time. */
      if (short_query_remaining_time(getpid()) > 5100 ||
          short_query_remaining_time(getpid()) < 5050) {
        printf("[Test 8]: ERROR: father remaining time is %d, should be around 5100!\n",
               short_query_remaining_time(getpid()));
        error = 1;
      }

      /* Wait for child to get it's return code. */
      waitpid(child_pid, &child_status, 0);

      return (error || child_status);
    } else {
      /* This is the child: */

      /* Verify that we have around 4900 milliseconds of run time. */
      if (short_query_remaining_time(getpid()) > 4900 ||
          short_query_remaining_time(getpid()) < 4850) {
        printf("[Test 8]: ERROR: child remaining time is %d, should be around 4900!\n",
               short_query_remaining_time(getpid()));
        error = 1;
      }

      /* Verify that the child has around 4900 milliseconds of requested time. */
      sched_getparam(getpid(), &params);
      if (params.sched_priority > 4900 || params.sched_priority < 4850) {
        printf("[Test 8]: ERROR: child requested time is %d, should be around 4900!\n",
               params.sched_priority);
        error = 1;
      }
      
      /* Run until overdue. */
      start_time = get_current_time();
      while (get_current_time() - start_time < 5.0) {
        a = a + b;
        b = a;
      }
      
      return error;
    }
  }

  return 0;
}

int test9() {
  int a = 1, b = 1;
  struct sched_param params = { 100 };
  double start_time, current_time;
  int child_pid;
  int child_status;
  int a_pid, b1_pid, b2_pid;
  int error = 0;
  
  printf("[Test 9]: Starting Run:\n");
  fflush(stdout);
  
  if (a_pid = fork()) {
    /* this is the father. */
    
    /* Set A as short and wait for it to become really overdue. */
    sched_setscheduler(a_pid, 4, &params);
    sleep(10);
    
    if (b1_pid = fork()) {
      /* this is the father. */

      /* Set B1 process to be short and wait for it to be 1 second overdue. */
      sched_setscheduler(b1_pid, 4, &params);
      sleep(1);

      if (b2_pid = fork()) {
        /* this is the father. */
        sched_setscheduler(b2_pid, 4, &params);
        
        /* Set B2 to be short and Wait for it to become 1 second overdue. */
        sleep(1);
      
      } else {
        /* this is process B2. */

        /* We work for 4 second. */
        start_time = get_current_time();
        while (get_current_time() - start_time < 4.1) {
          a = a + b;
          b = a;
        }

        return 0;
      }
    } else {
      /* this is process B1. */

      /* First we work for 1 second. */
      start_time = get_current_time();
      while (get_current_time() - start_time < 1.1) {
        a = a + b;
        b = a;
      }

      /* Now we spawn a child. */
      if ((child_pid = fork()) == 0) {
        /* This is the silly child. */

        /* Check the child overdue time is correct. */
        if (short_query_overdue_time(getpid()) > 5550 ||
            short_query_overdue_time(getpid()) < 5450) {
          printf("[Test 9]: ERROR: B1 child overdue time is %d, should be around 5500!\n",
                 short_query_overdue_time(getpid()));
          return 1;
        } else {
          return 0;
        }
      }

      /* Now we work for 2 second. */
      start_time = get_current_time();
      while (get_current_time() - start_time < 2) {
        a = a + b;
        b = a;
      }

      /* Collect child results. */
      waitpid(child_pid, &child_status, 0);
      return WEXITSTATUS(child_status);
    }
  } else {
    /* this is process A. */

    /* We do work for 10 seconds. */
    start_time = get_current_time();
    while (get_current_time() - start_time < 20) {
      a = a + b;
      b = a;
    }

    return 0;
  }

  /* Now wait for the processes to finish. */
  child_pid = wait(NULL);
  if (child_pid != b2_pid) {
    printf("[Test 9]: ERROR: First child to return should be B2, instead of %s!\n",
           ((child_pid == b1_pid) ? "B1" : ((child_pid == a_pid) ? "A" : "Unknown")));
    error = 1;
  }
  
  child_pid = wait(&child_status);
  if (child_pid != b1_pid) {
    printf("[Test 9]: ERROR: Second child to return should be B1, instead of %s!\n",
           ((child_pid == b2_pid) ? "B2" : ((child_pid == a_pid) ? "A" : "Unknown")));
    error = 1;
  }
  error = (error || WEXITSTATUS(child_status));
  
  child_pid = wait(NULL);
  if (child_pid != a_pid) {
    printf("[Test 9]: ERROR: Third child to return should be A, instead of %s!\n",
           ((child_pid == b1_pid) ? "B1" : ((child_pid == b2_pid) ? "B2" : "Unknown")));
    error = 1;
  }

  if (!error) {
    printf("[Test 9]: PASSED!\n");
  }
  
  return 0;
}

int test10() {
  int a, b;
  int i;
  int child_pid;
  double start_time, current_time;
  struct sched_param params;
  int order_of_return[10];
  int correct_order[] = { 7, 5, 3, 1, 9, 11, 13, 15, 17, 19 };
  int error = 0;

  printf("[Test 10]: Starting Run:\n");
  fflush(stdout);
  
  /* Spawn all children. */
  for (i = 19; i > 0; i -= 2) {
    if (fork() == 0) {
      /* This is the child: */

      /* Set child process to be short. */
      params.sched_priority = 8000;
      sched_setscheduler(getpid(), 4, &params);

      /* Start working for i seconds. */
      start_time = get_current_clock();
      while (get_current_clock() - start_time < ((double)i)) {
        a = a + b;
        b = a;
      }

      /* Return the child identifier. */
      return i;
    }
  }

  /* Wait for children to finish. */
  for (i = 0; i < 10; ++i) {
    wait(&(order_of_return[i]));
    order_of_return[i] = WEXITSTATUS(order_of_return[i]);
    if (order_of_return[i] != correct_order[i]) {
      error = 1;
    }
  }

  /* Print results. */
  if (error) {
    printf("[Test 10]: ERROR: Incorrect order of return.\n");
    printf("Correct Order : %d", correct_order[0]);
    for (i = 1; i < 10; ++i) {
      printf(", %d", correct_order[i]);
    }
    printf("\nReturned Order: %d", order_of_return[0]);
    for (i = 1; i < 10; ++i) {
      printf(", %d", order_of_return[i]);
    }
    printf("\n");
  } else {
    printf("[Test 10]: PASSED!\n");
  }

  return 0;
}

int main(int argc, char* argv[]) {
  int test_num = 0;
  
  if (argc != 2) {
    printf("ERROR: No test number given!\n");
    printf("Usage: ./staff_test <num_test>\n");
    printf("       Test Numbers: 2, 5, 7, 8, 9, 10\n");
    return 1;
  }

  test_num = atoi(argv[1]);

  switch(test_num) {
  case 2:
    return test2();
  case 5:
    return test5();
  case 7:
    return test7();
  case 8:
    return test8();
  case 9:
    return test9();
  case 10:
    return test10();
  default:
    printf("ERROR: Invalid test number!\n");
    printf("Usage: ./staff_test <num_test>\n");
    printf("       Test Numbers: 2, 5, 7, 8, 9, 10\n");
    return 1;
  }
}
