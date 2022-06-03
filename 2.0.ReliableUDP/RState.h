#ifndef __RSTATE__H__
#define __RSTATE__H__

/*
 * ather
 * The University of Queensland, 2022
 */

#include "RQueue.h"

#define NSTATES 2U

enum progress {
	MYS = 0U,
	HIS = 1U
};

typedef struct RState_t {
	RHead_t		states[NSTATES];
	Entry		curr;
	RHdr_p		request;
	uint8_t 	naks;
	uint16_t	ack;
	uint16_t	seq;
	bool		chksum;
} RState_t;
typedef RState_t *RState_p;

#define RSTATE_INIT(rstate) \
	RHEAD_INIT(rstate.states[MYS]); \
	RHEAD_INIT(rstate.states[HIS]); \
	rstate.request = NULL; \
	rstate.curr = NULL; \
	rstate.naks = 0; \
	rstate.seq = 1; \
	rstate.chksum = false;

void
rstate_destroy(RState_p rstate)
{
	rhead_destroy(&rstate->states[MYS]);
	rhead_destroy(&rstate->states[HIS]);
	rstate->request = NULL;
	rstate->curr = NULL;
	rstate->seq = 0;
	rstate->ack = 0;
	rstate->chksum = false;
}

void
rstate_show(RState_p rstate)
{
	printf("MYS\n");
	rhead_traverse(&rstate->states[MYS], entry_show);
	printf("HIS\n");
	rhead_traverse(&rstate->states[HIS], entry_show);
}

void
rstate_whitetest_run(void)
{
	RState_t rstate;
	RSTATE_INIT(rstate);

	rhead_whitetest(&rstate.states[MYS]);
	rhead_whitetest(&rstate.states[HIS]);

	rstate_destroy(&rstate);
}

#endif //__RSTATE__H__
