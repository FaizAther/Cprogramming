#ifndef __RDSTPKT__H__
#define __RDSTPKT__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdint.h>
#include <math.h>

#include "RSPkt.h"

typedef struct RDstPktS_t {
	struct RSPkt_t rs_pkt;
	uint16_t latitude;
	uint16_t longitude;
} RDstPktS_t;
typedef RDstPktS_t *RDstPktS;

typedef struct RDstPktB_t {
	struct RSPkt_t rs_pkt;
	uint32_t target_ip;
	uint32_t distance;
} RDstPktB_t;
typedef RDstPktB_t *RDstPktB;

void
rDstS_convert_(RDstPktS pkt, bool send)
{
	uint16_t (*foo)(uint16_t);
	if (send) { foo = &htons; }
	else { foo = &ntohs; }

	pkt->latitude = foo(pkt->latitude);
	pkt->longitude = foo(pkt->longitude);
}

void
rDstB_convert_(RDstPktB pkt, bool send)
{
	uint32_t (*foo)(uint32_t);
	if (send) { foo = &htonl; }
	else { foo = &ntohl; }

	pkt->distance = foo(pkt->distance);
}

static void
swap(uint16_t *a, uint16_t *b)
{
	uint16_t tmp;
	tmp = *a;
	*a = *b;
	*b = tmp;
}

uint32_t
rDst_calc_dist(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2)
{
	if (x1 > x2) { swap(&x1, &x2); }
	if (y1 > y2) { swap(&y1, &y2); }
	double s = (double)((uint64_t)(x2 - x1)*(x2 - x1) + (uint64_t)(y2- y1)*(y2- y1));
	return (uint32_t)sqrt(s);
}

void
rDst_Bshow(RDstPktB dst, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);
	char name[IP_MAX_NAME];

	rsPkt_show(&dst->rs_pkt, stderr);
	memset(name, 0, IP_MAX_NAME);
	rsPkt_ip_op_take(&dst->target_ip, name);
	fprintf(stream, \
		"\target_ip={%s, hex={0x%08x}, ptr={%p}}\n", \
		name, dst->target_ip, (void *)&dst->target_ip);
	if (dst->rs_pkt.mode == RS_LOCATION_BROADCAST) {
		fprintf(stream, "\tdistance={%u, %x}\n", dst->distance, dst->distance);
	}
	fprintf(stream, "end@%s }\n", __func__);
}

void
rDst_Sshow(RDstPktS dst, FILE *stream)
{
	fprintf(stream, "start@%s {\n", __func__);

	rsPkt_show(&dst->rs_pkt, stderr);
	fprintf(stream, "\tx={%u, %x}, y={%u, %x}\n", \
		dst->latitude, dst->latitude, \
		dst->longitude, dst->longitude);

	fprintf(stream, "end@%s }\n", __func__);
}

void
rDst_whitetest(void)
{
	RDstPktS_t rs;
	printf("sizeof(RDstPktS_t)={%lu}\n", sizeof(rs));
	printf("sqrt(465-2)^2+(784-5)^2)={%u}\n", rDst_calc_dist(2, 465, 5, 784));
}

#endif //__RDSTPKT__H__