#ifndef __RSRVTCP_H__
#define __RSRVTCP_H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <arpa/inet.h>
#include <sys/time.h>
#include <stdbool.h>

#include "RSockCom.h"
#include "RIPop.h"
#include "RSPkt.h"
#include "RDatPkt.h"
#include "RGreet.h"
#include "RStr.h"
#include "RTcp.h"
#include "RMap.h"
#include "RTabel.h"
// #include "RSrvUdp.h"

#define RSRV_TCP_IP_CHECK "44.45.46.1"

typedef struct RSwitch_t {
	RSock connection;
	RIPsingle_t ip_box;
	char ip_name[IP_MAX_NAME];
	char buf[RSOCK_BUF_MAX_SZ];
	RStr_lst_t mailbox;
	time_t query;
	RGreet_t assign;
	uint32_t distance;
	bool conlist;
	struct RSwitch_t *next;
} RSwitch_t;
typedef RSwitch_t *RSwitch;

typedef struct RSrvTcp_t {
	RIPop_t ip_op;
	struct RSwitch_t *head;
	struct RSwitch_t *tail;
	struct RSwitch_t *head_con;
	struct RSwitch_t *tail_con;
	uint64_t switch_n;
	uint64_t switchcon_n;
	char name[IP_MAX_NAME];
	uint16_t x;
	uint16_t y;
	RMap_t map;
	RTcp tbox;
	struct RSrvUdp_t *sup;
	struct RTabel_t tabel;
} RSrvTcp_t;
typedef RSrvTcp_t *RSrvTcp;

bool
rsSvrTcp_route_handle(RSrvTcp stp, RData pkt, RStr str);

static RSwitch
rsSvrTcp_find_switch_ip(RSwitch head, uint32_t find_ip, RSock con)
{
	char dest_ip[IP_MAX_NAME];
	rsPkt_ip_op_take(&find_ip, dest_ip);
	fprintf(stderr, "SEND TO %s\n", dest_ip);

	for (RSwitch curr = head; curr != NULL; curr = curr->next) {
		if (con) {
			if (memcmp(curr->buf, &con->addr, sizeof(struct sockaddr_in)) == 0) {
				return curr;
			}
		} else {
			if (strncmp(curr->ip_name, dest_ip, IP_MAX_NAME) == 0) {
				fprintf(stderr, "FOUND %s\n", dest_ip);
				return curr;
			}
		}
	}
	return NULL;
}

RSwitch
rSrvTcp_switch_append(RSrvTcp stp, RSock client, bool con)
{
	RSwitch rswitch = (RSwitch)malloc(sizeof(RSwitch_t));
	memset(rswitch, 0, sizeof(RSwitch_t));
	rswitch->connection = client;

	if (con) {
		if (stp->head_con) {
			stp->tail_con->next = rswitch;
			stp->tail_con = rswitch;
		} else {
			stp->tail_con = stp->head_con = rswitch;
		}
		stp->switchcon_n += 1;
		rswitch->conlist = true;
	} else {
		rIpop_assign_ip(&stp->ip_op, &rswitch->ip_box, true);
		rIpop_op_to_str(&rswitch->ip_box, rswitch->ip_name);
		if (stp->head) {
			stp->tail->next = rswitch;
			stp->tail = rswitch;
		} else {
			stp->tail = stp->head = rswitch;
		}
		stp->switch_n += 1;
	}
	return rswitch;
}

static bool
rSvrTcp_init(RSrvTcp stp, char *name, int subnet)
{
	if (name == NULL || subnet < 0 || subnet > IP_MAX_SPACE) { return false; }
	uint8_t name_sz;
	memset(stp, 0, sizeof(struct RSrvTcp_t));
	RIPOP_INIT(&stp->ip_op);
	if (!rIpop_str_validate_good(name, &name_sz)) { return false; }
	memcpy(stp->name, name, name_sz);
	rIpop_str_to_op(&stp->ip_op, stp->name);
	rIpop_justify(&stp->ip_op, subnet);
	rIpop_show(&stp->ip_op, stderr);
	return true;
}

static void
rSrvTcp_whitetest(void)
{
	fprintf(stderr, "%s\n", __func__);
	RSrvTcp_t stt;
	RSrvTcp stp = &stt;
	if (!rSvrTcp_init(stp, RSRV_TCP_IP_CHECK, RIP_DEFAULT_SUBNET_CHECK)) { exit(EXIT_FAILURE); }
}

#endif //__RSRVTCP_H__