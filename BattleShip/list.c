//	Mohammad Faiz ATHER === UQ 2020
//
//	list.c
//	implementation of a List using Thing Node
//
#include <stdlib.h>
#include <assert.h>

#include "list.h"
#include "node.h"

typedef struct _list {
	Node	head;
	Node	tail;
	size_t	size;
} _list;

List
list_new (void)
{
	List l = (List)malloc ( sizeof (*l) );
	assert (l != NULL);
	(*l).head = (*l).tail = NULL;
	(*l).size = 0;
	return l;
}

void
list_append (List l, Thing t)
{
	assert (l != NULL);
	Node n = node_make (t);
	if (l->head == NULL)
	{
		l->head = l->tail = n;
	} else {
		node_set_next (l->tail, n);
		node_set_prev (n, l->tail);
		l->tail = n;
	}
	l->size++;
}

void
list_prepend (List l, Thing t)
{
	assert (l != NULL);
	Node n = node_make (t);
	if (l->head == NULL)
	{
		l->head = l->tail = n;
	} else {
		node_set_prev (l->head, n);
		node_set_next (n, l->head);
		l->head = n;
	}
	l->size++;
}

Thing
list_top (List l)
{
	assert (l != NULL);
	if (l->size == 0) return thing_null();
	Thing t = thing_copy ( node_get_thing (l->head) );
	Node f = l->head;
	if (node_get_next (l->head) != NULL) node_set_prev (node_get_next (l->head), NULL);
	else l->head = l->tail = NULL;
	l->head = node_get_next (f);
	node_destroy (f);
	l->size--;
	return t;
}

Thing
list_bottom (List l)
{
	assert (l != NULL);
	if (l->size == 0) return thing_null();
	Thing t = thing_copy ( node_get_thing (l->tail) );
	Node f = l->tail;
	if (node_get_prev (l->tail) != NULL) node_set_next (node_get_prev (l->tail) , NULL);
	else l->head = l->tail = NULL;
	l->tail = node_get_prev (f);
	node_destroy (f);
	l->size--;
	return t;
}

size_t
list_size (List l)
{
	assert (l != NULL);
	return l->size;
}

static void
list_destroy_h ( Node n )
{
    if (n == NULL)
    {
        return;
    }

    list_destroy_h ( node_get_next (n) );
    node_destroy (n);
}

void
list_destroy (List l)
{
    assert (l != NULL);
    list_destroy_h ( (*l).head );
    free (l);
}
