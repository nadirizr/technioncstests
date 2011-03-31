#include <unistd.h>
#include <stdio.h>

#include "../syscall_tags.h"

int main() {
  int len = 0;
  
  int pid = getpid();
  int tag = 10;
  int array[20];
  int i;

  printf("// Calling: gettag ...\n");
  printf("// tag = %d\n\n", gettag(pid));
  printf("// Calling: settag(10) ...\n");
  settag(pid, tag);
  printf("// Calling: gettag ...\n");
  printf("// tag = %d\n\n", gettag(pid));
  printf("// Calling: getgoodprocesses ...\n");
  printf("// count = %d\n\n", len = getgoodprocesses(array, 20));
  printf("// array = {");
  for (i = 0; i < len; ++i) {
    printf("%d, ", array[i]);
  }
  printf("}\n\n");
  printf("// Calling: makegoodprocesses ...\n");
  printf("// num_fixed = %d\n\n", makegoodprocesses());
  printf("// Calling: getgoodprocesses ...\n");
  printf("// count = %d\n", len = getgoodprocesses(array, 20));
  printf("// array = {");
  for (i = 0; i < len; ++i) {
    printf("%d, ", array[i]);
  }
  printf("}\n\n");
  
  return 0;
}
