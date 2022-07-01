#ifndef __RGREET__H__
#define __RGREET__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSPkt.h"

typedef struct RGreet_t {
	struct RSPkt_t rs_pkt;
	uint32_t assign_ip;
} RGreet_t;
typedef RGreet_t *RGreet;

#define RGREET_INIT(pkt) \
	RSPKT_INIT((pkt)); \
	(pkt)->assign_ip = 0;

void
rGreet_get_assign_ip(RGreet pkt, char name[IP_MAX_NAME])
{
	rsPkt_ip_op_take(&pkt->assign_ip, name);
}

void
rGreet_set_assign_ip(RGreet pkt, char *name)
{
	rsPkt_ip_op_put(&pkt->assign_ip, name);
}

static void
rGreet_show(RGreet pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);
	char name[IP_MAX_NAME];

	rsPkt_show(&pkt->rs_pkt, stderr);
	memset(name, 0, IP_MAX_NAME);
	rGreet_get_assign_ip(pkt, name);
	fprintf(stream, \
		"\tassign_ip={%s, hex={0x%08x}, ptr={%p}}\n", \
		name, pkt->assign_ip, (void *)&pkt->assign_ip);

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rGreet_show_size(RGreet pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	rsPkt_show_size(&pkt->rs_pkt, stderr);

	fprintf(stream, "\tpkt={%lu}\n", sizeof((*pkt)));
	fprintf(stream, \
		"\tassign_ip={%lu}, offset={%lu}\n", \
		sizeof(pkt->assign_ip), offsetof(RGreet_t, assign_ip));

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rGreet_whitetest(void)
{
	RGreet_t xpkt = {0}, xpkt_cpy = {0};
	RGreet pkt = &xpkt;
	RSPKT_INIT(&xpkt.rs_pkt);
	RSPKT_INIT(&xpkt_cpy.rs_pkt);
	RSPkt ppkt = &xpkt.rs_pkt;
	char name[IP_MAX_NAME] = {0};

	rGreet_set_assign_ip(pkt, "192.168.0.2");
	rGreet_show(pkt, stderr);

	rsPkt_whitetest(ppkt);

	rGreet_get_assign_ip(pkt, name);
	rGreet_show(pkt, stderr);
	rGreet_show_size(pkt, stderr);
}

#endif //__RGREET__H__