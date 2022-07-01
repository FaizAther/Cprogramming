#ifndef __RSPKT__H__
#define __RSPKT__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define UDP_MAX_PKT 1500

/* "127.000.000.001" */
#define IP_MAX_NAME 16

#define RSBlock_sz 4

#define RSNVals 3
enum rs_vals {
	SRC_IP = 0,
	DEST_IP,
	RES_MODE,
};

enum rs_mode {
	RS_DISCOVERY = 0x01,
	RS_OFFER = 0x02,
	RS_REQUEST = 0x03,
	RS_ACKNOWLEDGE = 0x04,
	RS_DATA_PAYLOAD = 0x05,
	RS_QUERY_PACKET = 0x06,
	RS_QUERY_ACCEPT = 0x07,
	RS_LOCATION_SEND = 0x08,
	RS_LOCATION_BROADCAST = 0x09,
	RS_MORE_FRAGMENTS = 0x0a,
	RS_LAST_FRAGMENT = 0x0b,
};

typedef struct RSPkt_t {
	uint32_t src_ip;
	uint32_t dest_ip;
	char reserved[RSBlock_sz - 1];
	uint8_t mode;
} RSPkt_t;
typedef RSPkt_t *RSPkt;

#define RSPKT_INIT(ppkt) \
	memset(ppkt, 0, sizeof((*ppkt)));

#define RSPKT_MODE_SET(ppkt, val) \
  ((ppkt)->mode) = val

#define RSPKT_MODE_CHK(ppkt, val) \
  ((((ppkt)->mode) == (val)) ? true : false)

/*
 *  Take the dname in network byte order (32 bits) and put it in name as 127.0.0.1
 */
static void
rsPkt_ip_op_take(uint32_t *dname, char name[IP_MAX_NAME])
{
	struct in_addr addr;
	addr.s_addr = *dname;
	memset(name, 0, IP_MAX_NAME);
	if (dname && name) {
		char *src = inet_ntoa(addr);
		memcpy(name, src, IP_MAX_NAME);
	}
}

/*
 *  Take the name as 127.0.0.1 and put in dname as network byte order (32 bits)
 */
static void
rsPkt_ip_op_put(uint32_t *dname, char *name)
{
	struct in_addr addr = {0};
	char name_m[IP_MAX_NAME] = {0};
	memset(name_m, 0, IP_MAX_NAME);
	memccpy(name_m, name, '\0', IP_MAX_NAME);
	if (dname) {
		addr.s_addr = inet_addr(name_m);
		*dname = addr.s_addr;
	}
}

void
rsPkt_get_dest_ip(RSPkt pkt, char name[IP_MAX_NAME])
{
	rsPkt_ip_op_take(&pkt->dest_ip, name);
}

void
rsPkt_get_src_ip(RSPkt pkt, char name[IP_MAX_NAME])
{
	rsPkt_ip_op_take(&pkt->src_ip, name);
}

void
rsPkt_set_dest_ip(RSPkt pkt, char *name)
{
	rsPkt_ip_op_put(&pkt->dest_ip, name);
}

void
rsPkt_set_src_ip(RSPkt pkt, char *name)
{
	rsPkt_ip_op_put(&pkt->src_ip, name);
}

static void
rsPkt_show_size(RSPkt pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	fprintf(stream, "\tpkt={%lu}\n", sizeof((*pkt)));
	fprintf(stream, "\tsource_ip={%lu}, offset={%lu}\n", \
		sizeof(pkt->src_ip), offsetof(RSPkt_t, src_ip));
	fprintf(stream, "\tdest_ip={%lu}, offset={%lu}\n", \
		sizeof(pkt->dest_ip), offsetof(RSPkt_t, dest_ip));
	fprintf(stream, "\treserved={%lu}, offset={%lu}\n", \
		sizeof(pkt->reserved), offsetof(RSPkt_t, reserved));
	fprintf(stream, "\tmode={%lu}, offset={%lu}\n", \
		sizeof(pkt->mode), offsetof(RSPkt_t, mode));

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rsPkt_convert_(RSPkt pkt, bool send, long start, long end, long jump)
{
	fprintf(stderr, "start@%s {\n", __func__);

	void *ppkt = (void *)(long)pkt;
	for (long i = start * jump; i < end * jump; i += jump) {
		uint32_t tmp;
		memcpy(&tmp, ppkt + i, RSBlock_sz);
		fprintf(stderr, "\t%s", send ? "ntohl" : "htonl");
		fprintf(stderr, " 32bits converting %p\n", ppkt + i);
		tmp = send ? ntohl(tmp) : htonl(tmp);
		memcpy(ppkt + i, &tmp, RSBlock_sz);
	}

	fprintf(stderr, "end@%s }\n", __func__);
}

void
rsPkt_convert_recv(RSPkt pkt)
{
	rsPkt_convert_(pkt, false, 2, RSNVals, RSBlock_sz);
}

void
rsPkt_convert_send(RSPkt pkt)
{
	rsPkt_convert_(pkt, true, 2, RSNVals, RSBlock_sz);
}

static void
rsPkt_show(RSPkt pkt, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	char name[IP_MAX_NAME];

	memset(name, 0, IP_MAX_NAME);
	rsPkt_get_src_ip(pkt, name);
	fprintf(stream, "\tsource_ip={%s, hex={0x%08x}, ptr={%p}}\n", \
		name, pkt->src_ip, (void *)&pkt->src_ip);
	memset(name, 0, IP_MAX_NAME);
	rsPkt_get_dest_ip(pkt, name);
	fprintf(stream, "\tdest_ip={%s, hex={0x%08x}, ptr={%p}}\n", \
		name, pkt->dest_ip, (void *)&pkt->dest_ip);
	fprintf(stream, "\treserved={%s, hex={0x%06x}, ptr={%p}}\n", \
		pkt->reserved, *(pkt->reserved), (void *)&pkt->reserved);
	fprintf(stream, "\tmode={%u, hex={0x%02x}, ptr={%p}}\n", \
		pkt->mode, pkt->mode, (void *)&pkt->mode);

	fprintf(stream, "end@%s }\n", __func__);
}

static void
rsPkt_whitetest(RSPkt p)
{
	RSPkt_t pkt = {0}, pkt_cpy = {0};
	bool chk;
	RSPKT_INIT(&pkt);
	RSPKT_INIT(&pkt_cpy);
	RSPkt ppkt = &pkt;
	if (p != NULL) {
		RSPKT_INIT(p);
		ppkt = p;
	}
	char name[IP_MAX_NAME];

	memset(name, 0, IP_MAX_NAME);
	memcpy(name, "45.10.10.111", IP_MAX_NAME);
	rsPkt_set_dest_ip(ppkt, name);

	memset(name, 0, IP_MAX_NAME);
	memcpy(name, "127.0.0.1", IP_MAX_NAME);
	rsPkt_set_src_ip(ppkt, name);

	RSPKT_MODE_SET(ppkt, RS_ACKNOWLEDGE);
	chk = RSPKT_MODE_CHK(ppkt, RS_ACKNOWLEDGE);
	fprintf(stderr, "RS_ACK ? %u\n", chk);
	chk = RSPKT_MODE_CHK(ppkt, RS_DISCOVERY);
	fprintf(stderr, "RS_DIS ? %u\n", chk);

	rsPkt_show_size(ppkt, stderr);
	rsPkt_show(ppkt, stderr);

	rsPkt_convert_send(ppkt);
	rsPkt_show(ppkt, stderr);

	rsPkt_convert_recv(ppkt);
	rsPkt_show(ppkt, stderr);
}

#endif //__RSPKT__H__