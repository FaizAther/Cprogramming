#ifndef SHARED_H
#define SHARED_H

#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>


char *attach_memory_block(char *, int);
bool detach_memory_block(char *);
bool destroy_memory_block(char *);

#define BLOCK_SIZE 4096
#define FILENAME "shared.mem"

#define SEM_PROD_FNAME "/prodlock"
#define SEM_CONS_FNAME "/conslock"

#endif //SHARED_H