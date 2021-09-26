#include <stdio.h>

#include "handle.h"
#include "queue.h"
#include "data.h"

#ifndef READ_H_
#define READ_H_

#define FILE_GAME       1
#define FILE_PLAYER_MAP 2
#define FILE_CPU_MAP    3
#define FILE_CPU_MOVES  4



/*
    description: creates a file by removing the 
    @params filename
    @return string
*/
Queue
simple_read (char *);

Data
read_files (char **);

void
read_destroy (Data d);

#endif