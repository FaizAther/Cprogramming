#ifndef __RDATA__H__
#define __RDATA__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSPkt.h"

#define RDATA_PAYLOAD_SZ \
	(UDP_MAX_PKT - sizeof(struct RSPkt_t))

typedef struct RData_t {
	struct RSPkt_t rs_pkt;
	char payload[RDATA_PAYLOAD_SZ];
} RData_t;
typedef RData_t *RData;

#define RDATA_INIT(pkt) \
	RSPKT_INIT(&(pkt)->rs_pkt); \
	RSPKT_MODE_SET(&(pkt)->rs_pkt, RS_DATA_PAYLOAD); \
	memset((pkt)->payload, 0, RDATA_PAYLOAD_SZ);

void
rData_get_payload(RData pkt, char payload[RDATA_PAYLOAD_SZ])
{
	memccpy(payload, pkt->payload, '\0', RDATA_PAYLOAD_SZ);
}

void
rData_set_payload(RData pkt, char *payload)
{
	memccpy(pkt->payload, payload, '\0', RDATA_PAYLOAD_SZ);
}

static void
rData_show(RData pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	rsPkt_show(&pkt->rs_pkt, stderr);
	fprintf(stream, \
		"\tpayload={%s\n\t, ...hex={0x%08x}, ptr={%p}}\n", \
		pkt->payload, *(pkt->payload), (void *)&pkt->payload);

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rData_show_size(RData pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	rsPkt_show_size(&pkt->rs_pkt, stderr);

	fprintf(stream, "\tpkt={%lu}\n", sizeof((*pkt)));
	fprintf(stream, \
		"\tpayload={%lu}, offset={%lu}\n", \
		sizeof(pkt->payload), offsetof(RData_t, payload));

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rData_whitetest(void)
{
	RData_t xpkt = {0}, xpkt_cpy = {0};
	RData pkt = &xpkt;
	RSPKT_INIT(&xpkt.rs_pkt);
	RSPKT_INIT(&xpkt_cpy.rs_pkt);
	RSPkt ppkt = &xpkt.rs_pkt;
	char name[RDATA_PAYLOAD_SZ] = {0};

	rData_set_payload(pkt, "HELLO, WORLD!");
	rData_show(pkt, stderr);

	rsPkt_whitetest(ppkt);

	RDATA_INIT(pkt);
	rData_set_payload(pkt, "HELLO, MOON!");
	rData_show(pkt, stderr);

	rData_get_payload(pkt, name);
	rData_show(pkt, stderr);
	rData_show_size(pkt, stderr);
}

#endif //__RDATA__H__