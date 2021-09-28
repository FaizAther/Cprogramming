//	Mohammad Faiz ATHER === UQ 2020
//
//	queue.h
//	an ADT for a generic Queue
//
#include "thing.h"

#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct _queue *Queue;


Queue
queue_init (void);

void
queue_destroy (Queue q);

Thing
queue_dequeue (Queue q);

void
queue_enqueue (Queue q, Thing t);

size_t
queue_size (Queue q);

char *
queue_to_string (Queue, char);

Queue
queue_to_queue (char *, char);

#endif
