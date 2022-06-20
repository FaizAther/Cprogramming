#ifndef __RMAP__H__
#define __RMAP__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <arpa/inet.h>

/* RMap Struct */

typedef struct RMDist_t {
	uint32_t to_ip;
	uint32_t distance;
	struct RMElem_t *holder; /* Points to the to_ip in the fromlist */
	struct RMDist_t *next;
} RMDist_t;
typedef RMDist_t *RMDist;

typedef struct RMElem_t {
	uint32_t from_ip;
	struct RMDist_t *headD;
	struct RMDist_t *tailD;
	struct RMElem_t *next;
	struct RMDist_t *currD;
	struct RMDist_t *foundD;
} RMElem_t;
typedef RMElem_t *RMElem;

typedef struct RMap_t {
	RMElem head;
	RMElem tail;
	RMElem curr;
	RMElem found;
} RMap_t;
typedef RMap_t *RMap;

/*
 * Returns number of bits that matched.
 */
uint8_t
rmap_longest_prefix_mathing(uint32_t a, uint32_t b)
{
	a = ntohl(a);
	b = ntohl(b);

	uint8_t matched = 0;
	for (int i = 31; i >= 0; i--) {
		bool x = false;
		uint32_t ax = (a & (uint32_t)(1 << i));
		uint32_t bx = (b & (uint32_t)(1 << i));
		if (ax == bx) { x = true; }
		if (!x) { return matched; }
		matched += 1;
	}
	return matched;
}

uint32_t
rMap_shortest_and_longest_matching_prefix_to(RMap map, uint32_t to_ip)
{
	uint32_t found_from_ip = 0;
	uint32_t found_from_ip_distance = 0;
	uint8_t found_from_ip_prefix = 0;

	for (map->curr = map->head; map->curr != NULL; map->curr = map->curr->next) { // traverse from list
		for (map->curr->currD = map->curr->headD; map->curr->currD != NULL; map->curr->currD = map->curr->currD->next) { // traverse to list inside from
			if (map->curr->currD->to_ip == to_ip) { // found
				if (found_from_ip == 0) { // save the from
					found_from_ip = map->curr->from_ip;
					found_from_ip_distance = map->curr->currD->distance;
				} else if (map->curr->currD->distance < found_from_ip_distance) { // smaller route found update
					found_from_ip = map->curr->from_ip;
					found_from_ip_distance = map->curr->currD->distance;
				} else if (map->curr->currD->distance == found_from_ip_distance) { // smallest prefix matching
					found_from_ip_prefix = rmap_longest_prefix_mathing(to_ip, found_from_ip);
					uint8_t tmp_prefix = rmap_longest_prefix_mathing(to_ip, map->curr->from_ip);
					if (found_from_ip_prefix < tmp_prefix) {
						found_from_ip = map->curr->from_ip;
						found_from_ip_distance = map->curr->currD->distance;
						found_from_ip_prefix = tmp_prefix;
					}
				}
			}
		}
	}
	return found_from_ip;
}

bool
rMap_find_to_ip(RMElem elem, uint32_t to_ip)
{
	elem->foundD = NULL;
	for (elem->currD = elem->headD; elem->currD != NULL; elem->currD = elem->currD->next) {
		if (elem->currD->to_ip == to_ip) { // found to in list
			elem->foundD = elem->currD;
			return true;
		}
	}
	return false;
}

bool
rMap_find_from_ip(RMap map, uint32_t from_ip)
{
	map->found = NULL;
	for (map->curr = map->head; map->curr != NULL; map->curr = map->curr->next) {
		if (map->curr->from_ip == from_ip) { // found from in list
			map->found = map->curr;
			return true;
		}
	}
	return false;
}

bool
rMap_append(RMap map, uint32_t from_ip, uint32_t to_ip, uint32_t distance)
{
	/* Empty */
	if (!map->head) {
		map->tail = map->head = (RMElem)malloc(sizeof(RMElem_t));
		memset(map->tail, 0, sizeof(RMElem_t));
		map->tail->from_ip = from_ip;

		map->tail->headD = map->tail->tailD = (RMDist)malloc(sizeof(RMDist_t));
		memset(map->tail->tailD, 0, sizeof(RMDist_t));
		map->tail->tailD->to_ip = to_ip;
		map->tail->tailD->distance = distance;

		map->tail->tailD->holder = map->tail->next = (RMElem)malloc(sizeof(RMElem_t));
		memset(map->tail->next, 0, sizeof(RMElem_t));
		map->tail = map->tail->next;
		map->tail->from_ip = to_ip;

		return true;
	}

	if (rMap_find_from_ip(map, from_ip)) { // from found (X)
		if (rMap_find_to_ip(map->found, to_ip) && map->found->foundD->distance > distance) { // to found (X)
			map->found->foundD->distance = distance;
			return true;
		} else { // to not found
			if (map->found->tailD == NULL) { // to headD tailD empty
				map->found->headD = map->found->tailD = (RMDist)malloc(sizeof(RMDist_t));
				memset(map->found->tailD, 0, sizeof(RMDist_t));
			} else { // non empty
				map->found->tailD->next = (RMDist)malloc(sizeof(RMDist_t));
				memset(map->found->tailD->next, 0, sizeof(RMDist_t));
				map->found->tailD = map->found->tailD->next;
			}
			map->found->tailD->to_ip = to_ip;
			map->found->tailD->distance = distance;
			RMElem tmp = map->found;
			if (rMap_find_from_ip(map, to_ip)) { // to found in from list (X)
				tmp->tailD->holder = map->found;
			} else { // to not found in the from list (X)
				tmp->tailD->holder = map->tail->next = (RMElem)malloc(sizeof(RMElem_t));
				memset(map->tail->next, 0, sizeof(RMElem_t));
				map->tail = map->tail->next;
				map->tail->from_ip = to_ip;
			}
			return true;
		}
	} else { // from not found
		map->tail->next = (RMElem)malloc(sizeof(RMElem_t));
		memset(map->tail->next, 0, sizeof(RMElem_t));
		map->tail = map->tail->next;
		map->tail->from_ip = from_ip;
		map->tail->headD = map->tail->tailD = (RMDist)malloc(sizeof(RMDist_t));
		memset(map->tail->tailD, 0, sizeof(RMDist_t));
		map->tail->tailD->to_ip = to_ip;
		map->tail->tailD->distance = distance;
		RMElem tmp = map->tail;
		if (rMap_find_from_ip(map, to_ip)) { // to found in from list (X)
			map->tail->tailD->holder = map->found;
		} else { // to not found in the from list (X)
			map->tail->tailD->holder = map->tail->next = (RMElem)malloc(sizeof(RMElem_t));
			memset(map->tail->next, 0, sizeof(RMElem_t));
			map->tail = map->tail->next;
			map->tail->from_ip = to_ip;
			tmp->tailD->holder = map->tail;
		}
		return true;
	}
	return false;
}

void
rMap_show(RMap map)
{
	fprintf(stderr, "RMap={%p} = [\n", (void *)map);
	for (map->curr = map->head; map->curr; map->curr = map->curr->next) {
		fprintf(stderr, "\tRElem={%p} = (from={%u}) [\n", (void *)map->curr, map->curr->from_ip);
		for (map->curr->currD = map->curr->headD; map->curr->currD; map->curr->currD = map->curr->currD->next) {
			fprintf(stderr, "\t\t(RDist={%p} => (to_ip={%u}, dist={%u}, @to_ip={%p}))\n", \
				(void *)map->curr->currD, map->curr->currD->to_ip, map->curr->currD->distance, (void *)map->curr->currD->holder);
		}
		fprintf(stderr, "\t]\n");
	}
	fprintf(stderr, "]\n");
}

RMap
rMap_init(void)
{
	RMap map = (RMap)malloc(sizeof(RMap_t));
	memset(map, 0, sizeof(RMap_t));
	return map;
}

void
rMap_whitetest(void)
{
	RMap map = rMap_init();
	bool ret = rMap_append(map, 192, 168, 10);
	rMap_show(map);
	ret = rMap_append(map, 192, 168, 5);
	rMap_show(map);
	ret = rMap_append(map, 192, 56, 7);
	rMap_show(map);
	ret = rMap_append(map, 99, 192, 6);
	rMap_show(map);
	ret = rMap_append(map, 192, 99, 50);
	rMap_show(map);
	ret = rMap_append(map, 33, 66, 2);
	rMap_show(map);
	ret = rMap_append(map, 66, 8, 44);
	rMap_show(map);
	ret = rMap_append(map, 56, 8, 44);
	rMap_show(map);
}

#endif //__RMAP__H__