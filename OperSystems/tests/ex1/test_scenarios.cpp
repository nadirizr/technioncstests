#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "test.h"
#include "syscall_tags.h"

std::string location;

bool testTagsAfterReboot_OK_To_Fail_If_Not_After_Reboot() {
  int swapper_tag = gettag(0);
  int init_tag = gettag(1);
  int current_tag = gettag(getpid());

  ASSERT_TRUE(swapper_tag == 0);
  ASSERT_TRUE(init_tag == 0);
  ASSERT_TRUE(current_tag == 0);

  return true;
}

bool testGetTagInputs() {
  ASSERT_TRUE(gettag(0) >= 0);
  ASSERT_TRUE(gettag(1) >= 0);
  ASSERT_TRUE(gettag(getpid()) >= 0);

  ASSERT_TRUE(gettag(-1) == -1);
  ASSERT_TRUE(errno = ESRCH);
  ASSERT_TRUE(gettag(100000) == -1);
  ASSERT_TRUE(errno = ESRCH);

  return true;
}

bool testSetTagInputs() {
  int old_tag = gettag(getpid());

  ASSERT_TRUE(settag(getpid(), 0) == 0);
  ASSERT_TRUE(settag(getpid(), old_tag) == 0);

  // All of these should be ESRCH errros, and should precede EINVAL whenever
  // they both happen.
  ASSERT_TRUE(settag(-1, 100) == -1);
  ASSERT_TRUE(errno == -ESRCH);
  ASSERT_TRUE(settag(-1, -1) == -1);
  ASSERT_TRUE(errno == -ESRCH);
  ASSERT_TRUE(settag(-100, 100) == -1);
  ASSERT_TRUE(errno == -ESRCH);

  // All of these should be EINVAL errros.
  ASSERT_TRUE(settag(getpid(), -1) == -1);
  ASSERT_TRUE(errno == -EINVAL);
  ASSERT_TRUE(settag(getpid(), -100) == -1);
  ASSERT_TRUE(errno == -EINVAL);
  ASSERT_TRUE(settag(getppid(), 10) == -1);
  ASSERT_TRUE(errno == -EINVAL);
  ASSERT_TRUE(settag(getppid(), -10) == -1);
  ASSERT_TRUE(errno == -EINVAL);
  ASSERT_TRUE(settag(0, 500) == -1);
  ASSERT_TRUE(errno == -EINVAL);
  ASSERT_TRUE(settag(1, 200) == -1);
  ASSERT_TRUE(errno == -EINVAL);

  return true;
}

bool testGetGoodProcessesInputs() {
  int array[1000];
  int array2[1000];
  int reply = 0;
  int found = 0;
  int i;
  void* bogus;
  int my_pid = getpid();
  int old_tag = gettag(getpid());

  for (i = 0; i < 1000; ++i) {
    array[i] = array2[i] = -1;
  }

  ASSERT_TRUE(settag(my_pid, 1) == 0);

  // Do one round.
  reply = getgoodprocesses(array, 1000);
  ASSERT_TRUE(reply > 0);
  ASSERT_TRUE(reply < 1000);
  found = -1;
  for (i = 0; i < reply; ++i) {
    // Check if the array returned by getgoodprocesses is sorted.
    ASSERT_TRUE((i == 0) || (array[i-1] < array[i]));
    if (array[i] == my_pid) {
      found = i;
    }
  }
  for (i = reply; i < 1000; ++i) {
    ASSERT_TRUE(array[i] == -1);
  }
  ASSERT_TRUE(found > 0);

  // Do a second round.
  ASSERT_TRUE(reply == getgoodprocesses(array2, reply));
  for (i = 0; i < reply; ++i) {
    ASSERT_TRUE(array[i] == array2[i]);
  }
  for (i = reply; i < 1000; ++i) {
    ASSERT_TRUE(array2[i] == -1);
  }

  // Do a third round, with less elements and make sure no elements other than
  // ours were touched.
  for (i = 1; i < reply; ++i) {
    array[i] = -1;
  }
  ASSERT_TRUE(1 == getgoodprocesses(array, 1));
  ASSERT_TRUE(array[0] == array2[0]);
  for (i = 1; i < 1000; ++i) {
    ASSERT_TRUE(array[i] == -1);
  }

  // Now hand out a few basic errors.
  ASSERT_TRUE(-1 == getgoodprocesses(NULL, 10));
  ASSERT_TRUE(errno == EINVAL);
  ASSERT_TRUE(-1 == getgoodprocesses(array, -1));
  ASSERT_TRUE(errno == EINVAL);
  ASSERT_TRUE(-1 == getgoodprocesses(array, 0));
  ASSERT_TRUE(errno == EINVAL);
  bogus = reinterpret_cast<void*>(30);
  ASSERT_TRUE(-1 == getgoodprocesses((int*)bogus, 10));
  ASSERT_TRUE(errno == EINVAL);
  bogus = (void*)&printf;
  ASSERT_TRUE(-1 == getgoodprocesses((int*)bogus, 10));
  ASSERT_TRUE(errno == EINVAL);

  return true;
}

bool testMakeGoodProcessesFixSwapperTag() {
  int swapper_tag = gettag(0);
  int init_tag = gettag(1);
  int new_tag = (swapper_tag < init_tag ? init_tag : swapper_tag) + 100;
  int num_parents = 0;
  int pid_array[10];
  int pid_count = 0;

  ASSERT_TRUE(settag(getpid(), new_tag) == 0);
  num_parents = makegoodprocesses();
  ASSERT_TRUE(num_parents > 0);
  ASSERT_TRUE(gettag(0) == (new_tag + num_parents - 1));
  ASSERT_TRUE(gettag(1) == (new_tag + num_parents - 2));

  pid_count = getgoodprocesses(pid_array, 10);
  ASSERT_TRUE(pid_count > 2);
  ASSERT_TRUE(pid_array[0] == 0);
  ASSERT_TRUE(pid_array[1] == 1);

  return true;
}

int main() {
	// initialize random number generator
	srand( time(NULL) );

  RUN_TEST(testTagsAfterReboot_OK_To_Fail_If_Not_After_Reboot);
  RUN_TEST(testMakeGoodProcessesFixSwapperTag);
  RUN_TEST(testGetTagInputs);
  RUN_TEST(testSetTagInputs);
  RUN_TEST(testGetGoodProcessesInputs);

  return 0;
}
