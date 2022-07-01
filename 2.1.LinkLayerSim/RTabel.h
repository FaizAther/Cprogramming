#ifndef __RTABEL__H__
#define __RTABEL__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct RTElm_t {
	// uint32_t src;
	uint32_t dst;
} RTElm_t;
typedef struct RTElm_t *RTElm;

typedef struct RTNode_t {
	void *client;
	struct RTElm_t *elm;
	struct RTNode_t *next;
} RTNode_t;
typedef struct RTNode_t *RTNode;

typedef struct RTabel_t {
	struct RTNode_t *head;
	struct RTNode_t *tail;
} RTabel_t;
typedef struct RTabel_t *RTabel;

void *
rTabel_find(RTabel tabel, uint32_t dst)
{
	for (RTNode curr = tabel->head; curr != NULL; curr = curr->next) {
		if (curr->elm->dst == dst) {
			return curr->client;
		}
	}
	return NULL;
}

void
rTabel_append(RTabel tabel, void *client, uint32_t dst)
{
	if (client == NULL) { return; }
	if (rTabel_find(tabel, dst) != NULL) { return; }

	RTNode node1 = (RTNode)malloc(sizeof(RTNode_t));
	memset(node1, 0, sizeof(RTNode_t));
	node1->client = client;

	RTElm elm1 = (RTElm)malloc(sizeof(RTElm_t));
	memset(elm1, 0, sizeof(RTElm_t));
	elm1->dst = dst;

	node1->elm = elm1;
	if (tabel->head == NULL) {
		tabel->head = tabel->tail = node1;
	} else {
		tabel->tail->next = node1;
		tabel->tail = node1;
	}
}

#endif //__RTABEL__H__