#ifndef __RPROCESS__H__
#define __RPROCESS__H__

#include <sys/stat.h>

#include "RHdr.h"
#include "RQueue.h"
#include "RState.h"

#define MAX_NAKS 30

#define HOSTING_ROOT "./"
#define HOSTING_ROOT_SZ 2

char EMPTY_BUF[MSG_SIZ] = {0};

bool
set_fname(Data fname[MSG_SIZ], Data msg[MSG_SIZ])
{
	int ret = ~0;
	struct stat dat = {0};

	strcpy(fname, HOSTING_ROOT);
	memcpy(fname + HOSTING_ROOT_SZ, msg, MSG_SIZ - 2);
	fname[MSG_SIZ - 1] = '\0';
	
	ret = stat(fname, &dat);
	if (ret != 0) {
		fprintf(stderr, "stat: %d %s, {%s}\n", ret, strerror(errno), fname);
		return false;
	}
	return true;
}

/*
 * Got a new request in rstate->states[HIS]
 */
int
rprocess_request(RState_p rstate)
{
	RHdr_p hdr_p = rhead_peak(&rstate->states[HIS]);
	char fname[MSG_SIZ] = {0};
	rstate->curr = rhead_peak_entry(&rstate->states[MYS]);

	convert_bytes(hdr_p, true);
#ifdef DEBUG
	rhdr_pretty_print(hdr_p);
#endif
	if (!(FLAG_CONFIRM(*hdr_p))) {
		goto INVALID_NAK;
	}
	if (rstate->curr != NULL) {
		if (memcmp(hdr_p->msg, EMPTY_BUF, MSG_SIZ) != 0) {
			goto BAD_CHK;
		}
	}
	if (FLAG_VALIDATE((*hdr_p), CHK) == true || (rstate->chksum == true)) {
		if (rstate->curr == NULL) {
			rstate->chksum = true;
			if (valid_chksum(hdr_p) == false) {
				goto WORSE_CHK;
			}
		} else if (FLAG_VALIDATE((*hdr_p), CHK) != true || client_chksum(hdr_p) == false) {
			goto BAD_CHK;
		}
		FLAG_UNSET((*hdr_p), CHK);
	}

	if (rstate->curr != NULL) {
		if (FLAG_VALIDATE((*hdr_p), NAK) == false && !(hdr_p->content[SEQN] == (rstate->seq) && hdr_p->content[ACKN] == (rstate->ack))) {
			goto RET_PKT;
		}
	} else {
		if (!(hdr_p->content[SEQN] == 1 && hdr_p->content[ACKN] == 0)) {
			return false;
		}
	}
	if (((FLAG_CHECK((*hdr_p), (DAT|NAK)) == true) || (FLAG_VALIDATE((*hdr_p), NAK) == true)) && \
		(rstate->curr != NULL)) {
			/*
			 * NAK before GET respond NAK
			 * NAK after GET respond ACK if seq <= old_head->curr->seq
			 * NAK after GET respond NAK if seq > old_head->curr->seq
			 */
			RHdr_p old_hdr = rhead_peak(&rstate->states[MYS]);
			if (hdr_p->content[ACKN] < old_hdr->content[SEQN] && hdr_p->content[ACKN] > 0) {
				/* BAD */
			}
			if (rstate->curr == NULL) {
				return false;
			}
			if (rstate->ack == hdr_p->content[ACKN]) {
				rstate->seq += 1;
				goto RET_PKT;
			}
			goto INVALID_NAK;
	}
	if ((FLAG_CHECK((*hdr_p), GET) == true) && rstate->curr == NULL) {
		if (set_fname(fname, hdr_p->msg) == false) {
			return false;
		}
		FILE *resource = fopen(fname, "r");
		if (resource == NULL) {
			return false;
		}
		Entry e = rstate->curr = rhead_append(&rstate->states[MYS]);
		FLAG_SET(e->hdr, DAT);
		fread(e->hdr.msg, 1, MSG_SIZ, resource);
		fclose(resource);
		rstate->request = hdr_p;
		rstate->ack += 1;
		rstate->seq += 1;
	} else if (FLAG_CHECK((*hdr_p), (ACK|DAT)) == true && \
		rstate->curr != NULL) {
		if (set_fname(fname, rstate->request->msg) == false) {
			return false;
		}
		FILE *resource = fopen(fname, "r");
		if (resource == NULL) {
			return false;
		}
		Entry e = rstate->curr = rhead_append(&rstate->states[MYS]);
		
		if (fseek(resource, rstate->ack * MSG_SIZ, SEEK_SET) < 0) {
			fprintf(stderr, "fseek: %d %s\n", errno, strerror(errno));
		}
		if (feof(resource) != 0) {
			FLAG_SET(e->hdr, FIN);
			fclose(resource);
			rstate->ack += 1;
			rstate->seq += 1;
			goto SND_PKT;
		}
		FLAG_SET(e->hdr, DAT);
		size_t read_bs = fread(e->hdr.msg, 1, MSG_SIZ, resource);
		if (read_bs == 0) {
			FLAG_VERSION_INIT(e->hdr);
			FLAG_SET(e->hdr, FIN);
			fclose(resource);
			rstate->ack += 1;
			rstate->seq += 1;
			goto SND_PKT;
		}
		fclose(resource);
		rstate->ack += 1;
		rstate->seq += 1;
	} else if (FLAG_CHECK((*hdr_p), (ACK|FIN)) == true && \
		rstate->curr != NULL && \
		(FLAG_CHECK((rstate->curr->hdr), FIN) == true || FLAG_CHECK((rstate->curr->hdr), (FIN|CHK)) == true)) {
		Entry e = rstate->curr = rhead_append(&rstate->states[MYS]);
		FLAG_SET(e->hdr, ACK);
		FLAG_SET(e->hdr, FIN);
		if (FLAG_CHECK((*hdr_p), CHK) == true) {
			FLAG_SET(rstate->curr->hdr, CHK);
			rstate->curr->hdr.content[CHKS] = rhdr_computer_chksum(&rstate->curr->hdr);
		}
		if (FLAG_CHECK((*hdr_p), ENC) == true) {
			FLAG_SET(rstate->curr->hdr, ENC);
			encrypt(&rstate->curr->hdr);
		}
		rstate->curr->hdr.content[ACKN] = rstate->seq;
		rstate->ack += 1;
		rstate->seq += 1;
		if (rstate->chksum == true) {
			FLAG_SET(rstate->curr->hdr, CHK);
			rstate->curr->hdr.content[CHKS] = rhdr_computer_chksum(&rstate->curr->hdr);
		}
#ifdef DEBUG_END
		rhdr_pretty_print(&rstate->curr->hdr);
		printf("end\n");
#endif //DEBUG_END
		return false;
	} else {
INVALID_NAK:
		if (FLAG_VALIDATE((*rhead_peak(&rstate->states[HIS])), NAK)) {
			goto SND_PKT;
		} else {
			if (rstate->curr == NULL) {
				goto WORSE_CHK;
			} else {
				goto BAD_CHK;
			}
		}
	}
	if (rstate->curr == NULL) {
		goto WORSE_CHK;
	}
SND_PKT:
	if (rstate->curr != NULL && rstate->chksum == true && FLAG_VALIDATE(rstate->curr->hdr, CHK) == false) {
		FLAG_SET(rstate->curr->hdr, CHK);
		rstate->curr->hdr.content[CHKS] = rhdr_computer_chksum(&rstate->curr->hdr);
	}
RET_PKT:
	if (rstate->curr == NULL) {
		goto WORSE_CHK;
	}
#ifdef DEBUG
	if (!(FLAG_CONFIRM(rstate->curr->hdr))) {
		fprintf(stderr, "ERR: %s\n", __func__);
		_rhdr_pretty_print(&rstate->curr->hdr, stderr);
	}
	rhdr_pretty_print(&rstate->curr->hdr);
#endif
	//sleep(2);
	return true;
BAD_CHK:
	return 2;
WORSE_CHK:
	return 3;
}

#endif //__RPROCESS__H__
