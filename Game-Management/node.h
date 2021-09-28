//	Mohammad Faiz ATHER === UQ 2020
//
//	node.h
//	an ADT for a Thing Node
//
#include "thing.h"

#ifndef NODE_H_
#define NODE_H_

typedef struct _node *Node;


Node
node_make (Thing t);

Node
node_new (void);

Thing
node_get_thing (Node n);

void
node_set_thing (Node n, Thing t);

Node
node_get_prev (Node n);

Node
node_get_next (Node n);

void
node_set_prev (Node n, Node m);

void
node_set_next (Node n, Node m);

void
node_destroy (Node n);

#endif
