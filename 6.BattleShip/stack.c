//	Mohammad Faiz ATHER === UQ 2020
//
//	stack.c
//	an impelentation for a generic Stack using List
//
#include <stdlib.h>
#include <assert.h>

#include "stack.h"
#include "list.h"

typedef struct _stack {
    List l;
} _stack;


Stack
stack_init (void)
{
    Stack s = (Stack)malloc (sizeof (*s));
    assert (s != NULL);
    s->l = list_new ();
    return s;
}

void
stack_destroy (Stack s)
{
    assert (s != NULL);
    list_destroy (s->l);
    free (s);
}

Thing
stack_pop (Stack s)
{
    assert (s != NULL);
    return list_bottom (s->l);
}

void
stack_push (Stack s, Thing t)
{
    assert (s != NULL);
    list_append (s->l, t);
}

size_t
stack_size (Stack s)
{
    assert (s != NULL);
    return list_size (s->l);
}
