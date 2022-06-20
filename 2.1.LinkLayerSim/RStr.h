#ifndef __RSTR__H__
#define  __RSTR__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSPkt.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct RStr_t {
	char *buf_extra;
	uint32_t buf_extra_sz;
	bool excess_on;
	struct RStr_t *next;
} RStr_t;
typedef RStr_t *RStr;

typedef struct RStr_lst_t {
	struct RStr_t *head;
	struct RStr_t *tail;
} RStr_lst_t;
typedef RStr_lst_t *RStr_lst;

bool
RStr_init(RStr s, uint32_t size)
{
	if (s->buf_extra = (char *)malloc(size), s->buf_extra == NULL) { return false; }
	memset(s->buf_extra, 0, size);
	s->buf_extra_sz = size;
	s->excess_on = true;
	s->next = NULL;
	return true;
}

RStr
RStr_append(RStr s, uint32_t size)
{
	if (s == NULL) {
		s = (RStr)malloc(sizeof(RStr_t));
		RStr_init(s, size);
		return s;
	} else {
		s->next = (RStr)malloc(sizeof(RStr_t));
		RStr_init(s->next, size);
		return s->next;
	}
	return NULL;
}

void
RStr_lst_append(RStr_lst lst, uint32_t size)
{
	if (lst->head == NULL) {
		lst->head = lst->tail = RStr_append(NULL, size);
	} else {
		lst->tail->next = RStr_append(lst->tail, size);
		lst->tail = lst->tail->next;
	}
}

void
RStr_lst_buf_add(RStr_lst lst, char *buf, ssize_t buf_sz, char *stamp, size_t stamp_sz, uint32_t skip_sz)
{
	do {
		RStr_lst_append(lst, skip_sz);
		memcpy(lst->tail->buf_extra, stamp, stamp_sz);
		memcpy(lst->tail->buf_extra + stamp_sz, buf, ((ssize_t)(skip_sz - stamp_sz) > buf_sz ? (size_t)buf_sz : (skip_sz - stamp_sz)));
		buf = buf + (skip_sz - (ssize_t)stamp_sz);
		buf_sz = buf_sz - (skip_sz - stamp_sz);
		((RSPkt)lst->tail->buf_extra)->mode = RS_MORE_FRAGMENTS;
		fprintf(stderr, "%s: buf_sz={%ld}\n", __func__, buf_sz);
	} while (buf_sz > 0);
	if (lst->head->next) { ((RSPkt)lst->tail->buf_extra)->mode = RS_LAST_FRAGMENT; }
	else { ((RSPkt)lst->head->buf_extra)->mode = RS_DATA_PAYLOAD; }
}

static void
RStr_show_(RStr s, bool pkt, uint32_t pos)
{
	if (s == NULL) return;
	char *payload = s->buf_extra;
	fprintf(stderr, "RSTR_PKT={pos-%u, sz-%d}:\n", pos, s->buf_extra_sz);
	if (pkt) {
		rsPkt_show((RSPkt)s->buf_extra, stderr);
		payload += sizeof(RSPkt_t);
	}
	fprintf(stderr, "%s\n", payload);
	if (s->next != NULL) { RStr_show_(s->next, pkt, pos + 1); }
}

static void
RStr_show(RStr s, bool pkt)
{
	RStr_show_(s, pkt, 0);
}

void
RStr_destroy_(RStr s, uint32_t count, bool stack)
{
	fprintf(stderr, "%s %u", __func__, count);
	if (s->next != NULL) { RStr_destroy_(s->next, count + 1, stack); }
	if (s->buf_extra != NULL) { free(s->buf_extra); }
	s->excess_on = false;
	s->buf_extra_sz = 0;
	s->buf_extra = NULL;
	s->next = NULL;
	if (!stack) { free(s); }
}

void
RStr_destroy(RStr s, bool stack)
{
	RStr_destroy_(s, 0, stack);
}

void
RStr_whitetest(void)
{
	RStr_lst_t str_lst;

	char buf[4500];
	for (uint32_t i = 0; i < 4500; i++) {
		buf[i] = 'a';
	}

	RStr_lst_buf_add(&str_lst, buf, 4500, 0, 0, 1500);
	RStr_show(str_lst.head, false);
	RStr_destroy(str_lst.head, false);

	memset(&str_lst, 0, sizeof(str_lst));

	char stamp[12];
	for (uint32_t i = 0; i < 12; i++) {
		stamp[i] = 'H';
	}

	RStr_lst_buf_add(&str_lst, buf, 4500, stamp, 12, 1500);
	RStr_show(str_lst.head, false);
	RStr_destroy(str_lst.head, false);

}

#endif //__RSTR__H__