#include <stdlib.h>
#include <string>

#include "test.h"
#include "syscall_tags.h"

std::string location;

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

  RUN_TEST(testMakeGoodProcessesFixSwapperTag);

  return 0;
}
