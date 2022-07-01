#ifndef __RSRVUDP_H__
#define __RSRVUDP_H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "RSockCom.h"
#include "RIPop.h"
#include "RSPkt.h"
#include "RDatPkt.h"
#include "RGreet.h"
#include "RUdp.h"
#include "RStr.h"
// #include "RSrvTcp.h"

#define RSRV_UDP_IP_CHECK RIP_DEFAULT_IP_CHECK

typedef struct RAdaptor_t {
	struct sockaddr_in addr;
	socklen_t addr_len;
	RIPsingle_t ip_box;
	char ip_name[IP_MAX_NAME];
	char buf[RSOCK_BUF_MAX_SZ];
	RStr_lst_t mailbox;
	time_t query;
	struct RAdaptor_t *next;
} RAdaptor_t;
typedef RAdaptor_t *RAdaptor;

typedef struct RSrvUdp_t {
	RIPop_t ip_op;
	struct RAdaptor_t *head;
	struct RAdaptor_t *tail;
	uint64_t adaptor_n;
	char name[IP_MAX_NAME];
	RUdp ubox;
	struct RSrvTcp_t *stp;
} RSrvUdp_t;
typedef RSrvUdp_t *RSrvUdp;

bool
rSrvUdp_adaptor_append(RSrvUdp sup, RSock client)
{
	RAdaptor adaptor = (RAdaptor)malloc(sizeof(RAdaptor_t));
	memset(adaptor, 0, sizeof(RAdaptor_t));
	rIpop_assign_ip(&sup->ip_op, &adaptor->ip_box, true);
	client->addr_sz = sizeof(struct sockaddr_in);
	memcpy(&adaptor->addr, &client->addr, client->addr_sz);
	memcpy(adaptor->buf, client->buf, RSOCK_BUF_MAX_SZ);

	rIpop_op_to_str(&adaptor->ip_box, adaptor->ip_name);

	if (sup->head) {
		sup->tail->next = adaptor;
		sup->tail = adaptor;
	} else {
		sup->tail = sup->head = adaptor;
	}
	sup->adaptor_n += 1;
	return true;
}

static RAdaptor
rsSvrUdp_find_adaptor_ip(RAdaptor head, uint32_t find_ip)
{
	char dest_ip[IP_MAX_NAME];
	rsPkt_ip_op_take(&find_ip, dest_ip);
	fprintf(stderr, "SEND TO %s\n", dest_ip);

	for (RAdaptor curr = head; curr != NULL; curr = curr->next) {
		if (strncmp(curr->ip_name, dest_ip, IP_MAX_NAME) == 0) {
			fprintf(stderr, "FOUND %s\n", dest_ip);
			return curr;
		}
	}
	return NULL;
}

void
rsSrvUdp_fragment(RStr_lst head, RStr buf, RSPkt stamp)
{
	fprintf(stderr, "ERR: %s\n", __func__);
	RStr_lst_buf_add(head, buf->buf_extra + sizeof(RSPkt_t), buf->buf_extra_sz - sizeof(RSPkt_t), (char *)stamp, sizeof(RSPkt_t), UDP_BUF_MAX_SZ);
	RStr_show(head->head, true);
}

static bool
rSvrUdp_init(RSrvUdp sup, char *name, int subnet)
{
	if (name == NULL || subnet < 0 || subnet > IP_MAX_SPACE) { return false; }
	uint8_t name_sz;
	memset(sup, 0, sizeof(struct RSrvUdp_t));
	RIPOP_INIT(&sup->ip_op);
	if (!rIpop_str_validate_good(name, &name_sz)) { return false; }
	memcpy(sup->name, name, name_sz);
	rIpop_str_to_op(&sup->ip_op, sup->name);
	rIpop_justify(&sup->ip_op, subnet);
	rIpop_show(&sup->ip_op, stderr);
	return true;
}

static void
rSrvUdp_whitetest(void)
{
	fprintf(stderr, "%s\n", __func__);
	RSrvUdp_t sut;
	RSrvUdp sup = &sut;
	if (!rSvrUdp_init(sup, RSRV_UDP_IP_CHECK, RIP_DEFAULT_SUBNET_CHECK)) { exit(EXIT_FAILURE); }
}

#endif //__RSRVUDP_H__