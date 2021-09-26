//	Mohammad Faiz ATHER === UQ 2020
//
//	queue.c
//	an impelentation for a generic Queue using List
//
#include <stdlib.h>
#include <assert.h>

#include "queue.h"
#include "list.h"

typedef struct _queue {
    List l;
} _queue;


Queue
queue_init (void)
{
    Queue q = (Queue)malloc (sizeof (*q));
    assert (q != NULL);
    q->l = list_new ();
    return q;
}

void
queue_destroy (Queue q)
{
    assert (q != NULL);
    list_destroy (q->l);
    free (q);
}

Thing
queue_dequeue (Queue q)
{
    assert (q != NULL);
    return list_top (q->l);
}

void
queue_enqueue (Queue q, Thing t)
{
    assert (q != NULL);
    list_append (q->l, t);
}

size_t
queue_size (Queue q)
{
    assert (q != NULL);
    return list_size (q->l);
}
