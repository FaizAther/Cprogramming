//	Mohammad Faiz ATHER === UQ 2020
//
//	stack.h
//	an ADT for a generic Stack
//
#include "thing.h"

#ifndef STACK_H_
#define STACK_H_

typedef struct _stack *Stack;

Stack
stack_init (void);

void
stack_destroy (Stack s);

Thing
stack_pop (Stack s);

void
stack_push (Stack s, Thing t);

size_t
stack_size (Stack s);

#endif
