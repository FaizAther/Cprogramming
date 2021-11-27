//	Mohammad Faiz ATHER === UQ 2020
//
//	node.c
//	an implementation for a Thing Node
//
#include <stdlib.h>
#include <assert.h>

#include "node.h"

typedef struct _node {
	Thing	data;
	Node	prev;
	Node	next;
} _node;


Node
node_make (Thing t)
{
	Node n = node_new ();
	node_set_thing (n, t);
	return n;
}

Node
node_new (void)
{
	Node n = (Node)malloc(sizeof(*n));
	assert (n != NULL);
	node_set_thing (n, thing_null () );
	node_set_prev (n, NULL);
	node_set_next (n, NULL);

	return n;
}

Thing
node_get_thing (Node n)
{
	assert (n != NULL);
	return (*n).data;
}

void
node_set_thing (Node n, Thing t)
{
	assert (n != NULL);
	(*n).data = thing_copy (t);
}

Node
node_get_prev (Node n)
{
	assert (n != NULL);
	return n->prev;
}

Node
node_get_next (Node n)
{
	assert (n != NULL);
	return n->next;
}

void
node_set_prev (Node n, Node m)
{
	assert (n != NULL);
	(*n).prev = m;
}


void
node_set_next (Node n, Node m)
{
	assert (n != NULL);
	(*n).next = m;
}

void
node_destroy (Node n)
{
	assert (n != NULL);
	thing_destroy (n->data);
	free(n);
}
