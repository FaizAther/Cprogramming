#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    printf("usage - %s", argv[0]);
    exit(1);
  }
  // remove sem
  sem_unlink(SEM_PROD_FNAME);
  sem_unlink(SEM_CONS_FNAME);

  // sem init
  sem_t *sem_prod = sem_open(SEM_PROD_FNAME, O_CREAT, 0660, 0);
  if (sem_prod == SEM_FAILED) {
    perror("semopen/prod");
    exit(1);
  }

  // sem init
  sem_t *sem_cons = sem_open(SEM_CONS_FNAME, O_CREAT, 0660, 1);
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
  while (true) {
    sem_wait(sem_prod);
    if (strlen(block) > 0) {
      char buf[BLOCK_SIZE];
      strncpy(buf, block, BLOCK_SIZE);
      printf("Read \"%s\"\n", buf);
      bool done = (strcmp(block, "quit") == 0);
      if (done) break;
    }
    sem_post(sem_cons);
  }
  sem_close(sem_cons);
  sem_close(sem_prod);
  detach_memory_block(block);
  return 0;
}
