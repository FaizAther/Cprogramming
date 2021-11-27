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

char *
queue_to_string (Queue q, char delem)
{
    if (queue_size (q) < 0)
        return NULL;
    
    Thing t = queue_dequeue (q);
    while (queue_size (q) > 0)
    {
        t = thing_add (t, queue_dequeue (q), delem);
    }
    return t;
}

Queue
queue_to_queue (char *str, char delem)
{
    int len = strlen (str)+1;
    Queue q = queue_init ();
    char *start = str;
    int j = -1;
    for (int i = 0; i <= len; i++)
    {j++;
        if (str[j] == delem || str[j] == '\0') 
        {
            str[j] = '\0';
            if (strlen(str) > 0) queue_enqueue (q, str);
            str = start + i+1;
            j = -1;
        }
        
    }
    return q;
}
