#ifndef __RSRVUHANDLE__H__
#define __RSRVUHANDLE__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSrvUdp.h"

#include "RSrvTcp.h"

bool
rsSvrUdp_handle(RSrvUdp sup, RSock client)
{
	fprintf(stderr, "ERR: %s\n", __func__);
	RSPkt pkt = (RSPkt)client->buf;
	if (client->excess.excess_on) { pkt = (RSPkt)client->excess.buf_extra; }
	if (RSPKT_MODE_CHK(pkt, RS_DISCOVERY)) {
		fprintf(stderr, "DISCOVER\n");
		RIPsingle_t assign;
		char assign_ip[IP_MAX_NAME];
		rIpop_assign_ip(&sup->ip_op, &assign, false);
		rIpop_op_to_str(&assign, assign_ip);
		RGreet_t greet_pkt;
		RGREET_INIT(&greet_pkt);
		rGreet_set_assign_ip(&greet_pkt, assign_ip);
		rsPkt_set_src_ip((RSPkt)&greet_pkt, sup->ubox->ubox.host);
		RSPKT_MODE_SET((RSPkt)&greet_pkt, RS_OFFER);
		memcpy(pkt, &greet_pkt, sizeof(RGreet_t));
		fprintf(stderr, "SENDING THIS\n");
		rGreet_show((RGreet)pkt, stderr);
		rUdp_write(sup->ubox, client);
		pkt->dest_ip = greet_pkt.assign_ip;
		rSrvUdp_adaptor_append(sup, client);
	} else if (RSPKT_MODE_CHK(pkt, RS_REQUEST)) {
		RAdaptor dest_adaptor = rsSvrUdp_find_adaptor_ip(sup->head, ((RGreet)pkt)->assign_ip);
		if (dest_adaptor == NULL) { return false; }
		RGreet greet_pkt = (RGreet)dest_adaptor->buf;
		greet_pkt->rs_pkt.dest_ip = greet_pkt->assign_ip;
		RSPKT_MODE_SET((RSPkt)greet_pkt, RS_ACKNOWLEDGE);
		memcpy(pkt, greet_pkt, sizeof(RGreet_t));
		fprintf(stderr, "SENDING THIS\n");
		rGreet_show((RGreet)pkt, stderr);
		rUdp_write(sup->ubox, client);
		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_DATA_PAYLOAD)) {
		rsPkt_show(pkt, stderr);
		RAdaptor dest_adaptor = rsSvrUdp_find_adaptor_ip(sup->head, pkt->dest_ip);
		if (dest_adaptor == NULL) {
			rData_show((RData)pkt, stderr);
			rsSvrTcp_route_handle(sup->stp, (RData)pkt, &client->excess);
			return false;
		}
		RSock_t client_send;
		memset(&client_send, 0, sizeof(RSock_t));
		RSCom_clone(&client_send, &dest_adaptor->addr);
		RData_t dat_pkt;
		RDATA_INIT(&dat_pkt);
		dat_pkt.rs_pkt.src_ip = pkt->src_ip;
		dat_pkt.rs_pkt.dest_ip = pkt->dest_ip;
		if (client->excess.excess_on) { rsSrvUdp_fragment(&dest_adaptor->mailbox, &client->excess, (RSPkt)&dat_pkt); }
		else {
			memcpy(dat_pkt.payload, (client->buf + sizeof(RSPkt_t)), UDP_MAX_PKT - sizeof(RSPkt_t));
			RStr_lst_buf_add(&dest_adaptor->mailbox, (char *)&dat_pkt, sizeof(RData_t), 0, 0, UDP_BUF_MAX_SZ);
		}
		RSPkt_t craft;
		RSPKT_INIT(&craft);
		rsPkt_set_src_ip(&craft, sup->name);
		craft.dest_ip = pkt->dest_ip;
		RSPKT_MODE_SET(&craft, RS_QUERY_PACKET);
		memcpy(client_send.buf, &craft, sizeof(RSPkt_t));
		rsPkt_show(&craft, stderr);
		rUdp_write(sup->ubox, &client_send);
		rData_show(&dat_pkt, stderr);
	} else if (RSPKT_MODE_CHK(pkt, RS_QUERY_ACCEPT)) {
		rsPkt_show(pkt, stderr);
		RAdaptor dest_adaptor = rsSvrUdp_find_adaptor_ip(sup->head, pkt->src_ip);
		if (dest_adaptor == NULL) { return false; }
		RSock_t client_send;
		memset(&client_send, 0, sizeof(RSock_t));
		RSCom_clone(&client_send, &dest_adaptor->addr);
		RStr curr = dest_adaptor->mailbox.head;
		do {
			memcpy(client_send.buf, curr->buf_extra, sizeof(RData_t));
			rData_show((RData)client_send.buf, stderr);
			rUdp_write(sup->ubox, &client_send);
			curr = curr->next;
		} while (curr != NULL);
		RStr_destroy(dest_adaptor->mailbox.head, true);
		memset(&dest_adaptor->mailbox, 0, sizeof(RStr_lst_t));
		return true;
  } else {
		fprintf(stderr, "ERR: Not implemented\n");
  }
	return false;
}

#endif //__RSRVUHANDLE__H__