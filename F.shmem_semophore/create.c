#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

// #define BLOCK_SIZE 4096

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("usage - %s [stuff]]", argv[0]);
    exit(1);
  }

  // sem init
  sem_t *sem_prod = sem_open(SEM_PROD_FNAME, 0);
  if (sem_prod == SEM_FAILED) {
    perror("semopen/prod");
    exit(1);
  }

  // sem init
  sem_t *sem_cons = sem_open(SEM_CONS_FNAME, 0);
  if (sem_cons == SEM_FAILED) {
    perror("semopen/cons");
    exit(1);
  }

  // shared block
  char *block = attach_memory_block(FILENAME, BLOCK_SIZE);
  if (!block)
  {
    printf("ERROR\n");
    exit(1);
  }
  bool quit = false;
  for (int i = 0; i < 10 && !quit; ++i) {
    sem_wait(sem_cons);
    printf("Writing \"%s\"\n", argv[1]);
    if (strcmp(argv[1], "quit") == 0) quit = true;
    strncpy(block, argv[1], BLOCK_SIZE);
    sem_post(sem_prod);
  }
  sem_close(sem_cons);
  sem_close(sem_prod);
  detach_memory_block(block);
  return 0;
}
