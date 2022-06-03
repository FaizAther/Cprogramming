#ifndef __RQUEUE__H__
#define __RQUEUE__H__

#include <stdlib.h>
#include <sys/queue.h>

#include "RHdr.h"

typedef struct entry {
	struct RHdr_t	hdr;
	TAILQ_ENTRY(entry) entries;
} entry;
typedef struct entry *Entry;

typedef TAILQ_HEAD(_RHead_t, entry) RHead_t;
typedef RHead_t *RHead_p;

#define RHEAD_INIT(name) \
	name = (RHead_t)TAILQ_HEAD_INITIALIZER(name); \
	TAILQ_INIT(&name);

Data *
entry_puts(Entry e, Data *msg)
{
	size_t msg_len = strlen(msg);
	RHDR_MSG_SET(e->hdr, msg, msg_len);
	return msg_len < MSG_SIZ ? msg + msg_len : NULL;
}

Entry
rhead_append(RHead_p head)
{
	Entry e = (Entry)malloc(sizeof(entry));
	if (e == NULL) {
		fprintf(stderr, "malloc: %d %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	RHDR_INIT((e->hdr));
	if (TAILQ_EMPTY(head)) {
		RHDR_CONTENT_SET(e->hdr, SEQN, 1U);
	} else {
		RHDR_CONTENT_SET(e->hdr, SEQN, \
			(TAILQ_LAST(head, _RHead_t))->hdr.content[SEQN] + 1U);
	}
	TAILQ_INSERT_TAIL(head, e, entries);
	return e;
}

Entry
rhead_peak_entry(RHead_p head)
{
	return TAILQ_LAST(head, _RHead_t);
}

RHdr_p
rhead_peak(RHead_p head)
{
	return &TAILQ_LAST(head, _RHead_t)->hdr;
}

void
rhead_fix_seq(RHead_p head)
{
	Entry tail = rhead_peak_entry(head);
	Entry tail_prev = TAILQ_PREV(tail, _RHead_t, entries);
	if (tail->hdr.content[SEQN] != (tail_prev->hdr.content[SEQN] + 1)) {
		tail->hdr.content[SEQN] = tail_prev->hdr.content[SEQN] + 1;
	}
}

bool
rhead_check_seq(RHead_p head) {
	Entry tail = rhead_peak_entry(head);
	Entry tail_prev = TAILQ_PREV(tail, _RHead_t, entries);
	if (FLAG_CHECK(tail->hdr, NAK) == true || \
		tail_prev == NULL || tail == tail_prev) {
		return true;
	}
	if (tail->hdr.content[SEQN] == (tail_prev->hdr.content[SEQN] + 1) && \
		tail->hdr.content[ACKN] == (tail_prev->hdr.content[ACKN] + 1)) {
		return true;
	}
	return false;
}

void
rhead_destroy(RHead_p head)
{
	Entry var = TAILQ_FIRST(head);
	while (var != NULL) {
		Entry nxt = TAILQ_NEXT(var, entries);
		free(var);
		var = nxt;
	}
}
/*
 * Keeps the sequence at N and removes all after
 */
void
rhead_destroy_n_to_last(RHead_p head, uint32_t n)
{
	Entry var;
	TAILQ_FOREACH_REVERSE(var, head, _RHead_t, entries) {
		if (var->hdr.content[SEQN] < n)
			return;
		if (var->hdr.content[SEQN] > n) {
			TAILQ_REMOVE(head, var, entries);
		}
	}
}

void
entry_show(Entry e)
{
	rhdr_pretty_print(&(e->hdr));
}

void
rhead_traverse(RHead_p head, void (f(Entry)))
{
	uint64_t i = 0;
	Entry var;
	TAILQ_FOREACH(var, head, entries) {
		f(var);
		printf("RHeadEntry={%zu}\n", i);
		i++;
	}
}

void
rhead_whitetest(RHead_p head_p)
{
	Data buf[MSG_BUF];
	memset(buf, 'A', MSG_SIZ);
	buf[MSG_SIZ] = '\0';
	Entry e = rhead_append(head_p);
	entry_puts(e, buf);
	memset(buf, 'B', MSG_SIZ);
	buf[MSG_SIZ] = '\0';
	e = rhead_append(head_p);
	entry_puts(e, buf);
	memset(buf, 'C', MSG_SIZ);
	buf[MSG_SIZ] = '\0';
	e = rhead_append(head_p);
	entry_puts(e, buf);

	rhead_traverse(head_p, entry_show);
}

void
rhead_whitetest_run(void)
{
	RHead_t rhead;
	RHEAD_INIT(rhead);
	rhead_whitetest(&rhead);
	rhead_destroy_n_to_last(&rhead, 1);
	printf("-X-X-\n");
	rhead_traverse(&rhead, entry_show);
	rhead_destroy(&rhead);
}

#endif //__RQUEUE__H__
