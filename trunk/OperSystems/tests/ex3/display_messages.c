#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "mp_interface.h"

#define MAX_MESSAGE_SIZE 65536

int print_message(FILE* source_stream, int num) {
  /* First extract the message length and verify it's value is correct. */
  char message_data[MAX_MESSAGE_SIZE];
  int data_length = 0;
  int i;
  if (fread(&data_length, 2, 1, source_stream) <= 0) {
    if (feof(source_stream)) {
      return 1;
    }

    printf("ERROR [Message %d]: Couldn't read message length.\n", num);
    return -1;
  }
  if ((data_length < 4) || (data_length > MAX_MESSAGE_SIZE)) {
    printf("ERROR [Message %d]: Invalid data length: %d.\n", num, data_length);
    return -1;
  }

  /* Then read the actual message. */
  if (fread(message_data + 2, sizeof(char), data_length - 2,
      source_stream) <= 0) {
    printf("ERROR [Message %d]: Couldn't read message data.\n", num);
    return -1;
  }

  /* Then extract the message fields. */
  int target = (int)message_data[2];
  int is_urgent = ((message_data[3] & MSG_URGENT) ? 1 : 0);
  int is_final  = ((message_data[3] & MSG_FINAL ) ? 1 : 0);

  /* Finally print the message. */
  printf("================================================================\n");
  printf("[Message %d]:\n", num);
  printf("Total Length = %d\tTarget = %d\tFlags =%s%s%s\n",
         data_length, target,
         (is_urgent ? " URGENT" : ""),
         (is_final  ? " FINAL"  : ""),
         ((!is_final && !is_urgent) ? " None" : ""));
  for (i = 4; i < data_length; ++i) {
    printf("<%d>", (unsigned int)message_data[i]);
  }
  printf("\n================================================================\n");
  fflush(stdout);

  return 0;
}

int main(int argc, char* argv[]) {
  int i;
  int res = 0;
  FILE* file = NULL;

  if (argc != 2) {
    printf("Usage: ./dm <message file>\n");
    return 1;
  }

  file = fopen(argv[1], "rb");
  if (file == NULL) {
    printf("ERROR: Invalid file: '%s'\n", argv[1]);
    printf("Usage: ./dm <message file>\n");
    return 1;
  }

  for (i = 1; (res = print_message(file, i)) == 0; ++i) {};
  
  fclose(file);
  
  if (res < 0) {
    return 1;
  }
  return 0;
}
