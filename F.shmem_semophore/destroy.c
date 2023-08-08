#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

// #define BLOCK_SIZE 4096

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    printf("usage - %s]", argv[0]);
    exit(1);
  }
  if (destroy_memory_block(FILENAME)) {
    printf("Destroyed!\n");
  } else {
    printf("Error!\n");
  }
  return 0;
}
