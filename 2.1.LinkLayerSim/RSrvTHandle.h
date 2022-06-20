#ifndef __RSRVTHANDLE__H__
#define __RSRVTHANDLE__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSrvTcp.h"

#include "RDstPkt.h"

#include "RSrvUdp.h"

bool
rsSvrTcp_establish(RSrvTcp stp, RSock client)
{
	fprintf(stderr, "ERR: %s\n", __func__);
	int ret;
	RGreet_t gpkt;
	RGREET_INIT(&gpkt);
	RSPKT_MODE_SET((RSPkt)&gpkt, RS_DISCOVERY);
	memcpy(client->buf, &gpkt, sizeof(RGreet_t));

	if (ret = rTcp_write(client), ret < 0) { return false; }

	RSwitch rswitch = rSrvTcp_switch_append(stp, client, true);
	memcpy(rswitch->buf, &client->addr, sizeof(struct sockaddr_in));

	return true;
}

/*
 * Returns number of bits that matched.
 */
uint8_t
longest_prefix_mathing(uint32_t a, uint32_t b)
{
	a = ntohl(a);
	b = ntohl(b);

	uint8_t matched = 0;
	for (int i = 31; i >= 0; i--) {
		bool x = false;
		uint32_t ax = (a & (uint32_t)(1 << i));
		uint32_t bx = (b & (uint32_t)(1 << i));
		if (ax == bx) { x = true; }
		if (!x) { return matched; }
		matched += 1;
	}
	return matched;
}

void
rsSvrShowBuf(char *buf, char *from, RStr excess)
{
	long pos = ftell(stdout);
	int ret = 0;
	if (pos == -1) { // tty
		fprintf(stdout, "\b\b");
		fflush(stdout);
	} else { // file
		ret = fseek(stdout, pos - 2, SEEK_SET);
	}

	// TODO: mode of packet for showing received

	if (excess->excess_on) {
		RStr curr = excess;
		fprintf(stdout, "Received from %s: ", from);
		fflush(stdout);
		do {
			fprintf(stdout, "%s", curr->buf_extra + sizeof(RSPkt_t));
			fflush(stdout);
			curr = curr->next;
		} while (curr != NULL);
		fprintf(stdout, "\n> ");
		fflush(stdout);
	} else {
		fprintf(stdout, "Received from %.*s: %s\n> ", RSOCK_BUF_MAX_SZ - 12, from, buf);
		fflush(stdout);
	}
}

bool
rsSvrTcp_for_me(RSrvTcp stp, RData pkt, RStr excess)
{
	uint32_t my_ip = 0;
	char from[IP_MAX_NAME];
	memset(from, 0, sizeof(IP_MAX_NAME));
	rsPkt_ip_op_take(&pkt->rs_pkt.src_ip, from);

	rsPkt_ip_op_put(&my_ip, stp->name);
	if (pkt->rs_pkt.dest_ip == my_ip) { // TCP name
		fprintf(stderr, "ERR: pkt for me TCP\n");
		rsSvrShowBuf(pkt->payload, from, excess);
		return true;
	}
	if (stp->sup != NULL) {
		rsPkt_ip_op_put(&my_ip, stp->sup->name);
		if (pkt->rs_pkt.dest_ip == my_ip) { // UDP name
			fprintf(stderr, "ERR: pkt for me UDP\n");
			rsSvrShowBuf(pkt->payload, from, excess);
			return true;
		}
	}

	for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) {
		if (curr->assign.assign_ip == pkt->rs_pkt.dest_ip) { // in con
			fprintf(stderr, "ERR: pkt for me on con list\n");
			rsSvrShowBuf(pkt->payload, from, excess);
			return true;
		}
	}
	return false;
}

bool
rsSvrTcp_route_handle(RSrvTcp stp, RData pkt, RStr excess)
{
	fprintf(stderr, "ERR: %s\n", __func__);
	RSPkt_t snd;
	RData_t dat_pkt;
	uint32_t found_to_ip = 0;
	uint8_t max_matched = 0;

	/* TODO: If destination is me then print it out */
	if (rsSvrTcp_for_me(stp, pkt, excess)) {
		return false;
	}

	RSwitch sw_max_matched = stp->head_con;
	RSwitch tabel_find_sw = (RSwitch)rTabel_find(&stp->tabel, pkt->rs_pkt.dest_ip);
	if (tabel_find_sw != NULL) {
		sw_max_matched = tabel_find_sw;
		goto SND_PKT;
	}

	found_to_ip = rMap_shortest_and_longest_matching_prefix_to(&stp->map, pkt->rs_pkt.dest_ip);

	if (found_to_ip != 0) { // found something (send to it) list or con list?
		for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) {
			uint32_t his_ip = 0;
			rsPkt_ip_op_put(&his_ip, curr->ip_name);
			if (found_to_ip == his_ip) {
				sw_max_matched = curr;
				goto SND_PKT;
			}
		}
		for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) {
			if (curr->assign.rs_pkt.src_ip == found_to_ip) {
				sw_max_matched = curr;
				goto SND_PKT;
			}
		}
	}

	for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) { /* Longest matching prefix con list */
		uint8_t tmp_match = longest_prefix_mathing(curr->assign.rs_pkt.src_ip, pkt->rs_pkt.dest_ip);
		if (tmp_match > max_matched) {
			max_matched = tmp_match;
			sw_max_matched = curr;
		}
	}
	for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) { /* Longest matching prefix list */
		uint32_t ip = 0;
		rsPkt_ip_op_put(&ip, curr->ip_name);
		uint8_t tmp_match = longest_prefix_mathing(ip, pkt->rs_pkt.dest_ip);
		if (tmp_match > max_matched) {
			max_matched = tmp_match;
			sw_max_matched = curr;
		}
	}

	if (sw_max_matched == NULL) { return false; }

SND_PKT:

	memset(&snd, 0, sizeof(RSPkt_t));
	if (sw_max_matched->conlist) {
		snd.dest_ip = sw_max_matched->assign.rs_pkt.src_ip;
		snd.src_ip = sw_max_matched->assign.rs_pkt.dest_ip;
	} else {
		uint32_t ip = 0;
		rsPkt_ip_op_put(&ip, sw_max_matched->ip_name);
		snd.dest_ip = ip;
		rsPkt_ip_op_put(&ip, stp->name);
		snd.src_ip = ip;
	}

	RSPKT_MODE_SET(&snd, RS_QUERY_PACKET);

	memcpy(sw_max_matched->connection->buf, &snd, sizeof(RSPkt_t));

	rTcp_write(sw_max_matched->connection);

	RDATA_INIT(&dat_pkt);
	dat_pkt.rs_pkt.src_ip = pkt->rs_pkt.src_ip;
	dat_pkt.rs_pkt.dest_ip = pkt->rs_pkt.dest_ip;
	if (excess->excess_on) { rsSrvUdp_fragment(&sw_max_matched->mailbox, excess, (RSPkt)&dat_pkt); }
	else {
		memcpy(dat_pkt.payload, pkt->payload, UDP_MAX_PKT - sizeof(RSPkt_t));
		RStr_lst_buf_add(&sw_max_matched->mailbox, (char *)&dat_pkt, sizeof(RData_t), 0, 0, UDP_BUF_MAX_SZ);
	}

	return false;
}

bool
rsSvrTcp_handle(RSrvTcp stp, RSock client)
{
	fprintf(stderr, "ERR: %s\n", __func__);
	RSPkt pkt = (RSPkt)client->buf;
	char pkt_saved[RSOCK_BUF_MAX_SZ];
	memcpy(pkt_saved, client->buf, RSOCK_BUF_MAX_SZ);
	if (client->excess.excess_on) { pkt = (RSPkt)client->excess.buf_extra; }
	if (RSPKT_MODE_CHK(pkt, RS_DISCOVERY)) {
		fprintf(stderr, "DISCOVER\n");
		RIPsingle_t assign;
		char assign_ip[IP_MAX_NAME];
		rIpop_assign_ip(&stp->ip_op, &assign, false);
		rIpop_op_to_str(&assign, assign_ip);
		RGreet_t greet_pkt;
		RGREET_INIT(&greet_pkt);
		rGreet_set_assign_ip(&greet_pkt, assign_ip);
		rsPkt_set_src_ip((RSPkt)&greet_pkt, stp->tbox->tbox.host);
		RSPKT_MODE_SET((RSPkt)&greet_pkt, RS_OFFER);
		memcpy(pkt, &greet_pkt, sizeof(RGreet_t));
		fprintf(stderr, "SENDING THIS\n");
		rGreet_show((RGreet)pkt, stderr);
		// rUdp_write(sup->ubox, client);
		rTcp_write(client);
		pkt->dest_ip = greet_pkt.assign_ip;
		// rSrvUdp_adaptor_append(sup, client, false);
		RSwitch rswitch = rSrvTcp_switch_append(stp, client, false);
		memcpy(rswitch->buf, (RGreet)pkt, sizeof(RGreet_t));
		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_OFFER)) {
		int ret;
		RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head_con, ((RGreet)pkt)->assign_ip, client);
		RGreet ret_greet = (RGreet)client->buf;
		rGreet_show(ret_greet, stderr);
		memset(dest_switch->ip_name, 0, sizeof(dest_switch->ip_name));
		rGreet_get_assign_ip(ret_greet, dest_switch->ip_name);
		RIPop_t ip_trash;
		memset(&ip_trash, 0, sizeof(RIPop_t));
		rIpop_str_to_op(&ip_trash, dest_switch->ip_name);
		memcpy(&dest_switch->ip_box, &ip_trash.ip_box, sizeof(RIPsingle_t));

		RGreet_t gpkt;
		RGREET_INIT(&gpkt);
		gpkt.rs_pkt.dest_ip = ret_greet->rs_pkt.src_ip;
		gpkt.assign_ip = ret_greet->assign_ip;
		RSPKT_MODE_SET((RSPkt)&gpkt, RS_REQUEST);

		memcpy(client->buf, &gpkt, sizeof(RGreet_t));
		if (ret = rTcp_write(client), ret < 0) { return false; }


	} else if (RSPKT_MODE_CHK(pkt, RS_ACKNOWLEDGE)) {
		RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head_con, ((RGreet)pkt)->assign_ip, client); /*  WHICH LIST? */
		memcpy(&dest_switch->assign, pkt, sizeof(RGreet_t));
		RDstPktS_t dst_pkt;
		memset(&dst_pkt, 0, sizeof(RDstPktS_t));
		dst_pkt.rs_pkt.src_ip = pkt->dest_ip;
		dst_pkt.rs_pkt.dest_ip = pkt->src_ip;
		dst_pkt.rs_pkt.mode = RS_LOCATION_SEND;
		dst_pkt.latitude = stp->x;
		dst_pkt.longitude = stp->y;
		memcpy(client->buf, &dst_pkt, sizeof(RDstPktS_t));
		rDst_Sshow(&dst_pkt, stderr);
		rDstS_convert_((RDstPktS)client->buf, true);
		rTcp_write(client);
		memcpy(dest_switch->buf, &dst_pkt, sizeof(RDstPktS_t));
		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_LOCATION_SEND)) {
		RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head, (pkt)->src_ip, NULL); /*  WHICH LIST? */
		RSPkt prev_pkt = (RSPkt)dest_switch->buf;
		if (dest_switch && !RSPKT_MODE_CHK(prev_pkt, RS_LOCATION_SEND)) { /* REPLY TO CONNECTED TO ME peer */
			RDstPktS_t dst_pkt;
			memset(&dst_pkt, 0, sizeof(RDstPktS_t));
			dst_pkt.rs_pkt.src_ip = pkt->dest_ip;
			dst_pkt.rs_pkt.dest_ip = pkt->src_ip;
			dst_pkt.rs_pkt.mode = RS_LOCATION_SEND;
			dst_pkt.latitude = stp->x;
			dst_pkt.longitude = stp->y;
			memcpy(client->buf, &dst_pkt, sizeof(RDstPktS_t));
			rDst_Sshow(&dst_pkt, stderr);
			rDstS_convert_((RDstPktS)client->buf, true);
			rTcp_write(client);
			//RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head_con, ((RGreet)pkt)->assign_ip, client);
			memcpy(dest_switch->buf, &dst_pkt, sizeof(RDstPktS_t));
		}
		/* BROADCAST message */
		RDstPktS dst_got = (RDstPktS)pkt_saved; /* FROM CLIENT IN NETWORK BYTE ORDER */
		rDstS_convert_(dst_got, false);
		RDstPktB_t broadcast, broadcast_cpy;
		memset(&broadcast, 0, sizeof(RDstPktB_t));
		memset(&broadcast_cpy, 0, sizeof(RDstPktB_t));
		broadcast.rs_pkt.src_ip = dst_got->rs_pkt.dest_ip;
		broadcast.rs_pkt.dest_ip = dst_got->rs_pkt.src_ip;
		broadcast.target_ip = dst_got->rs_pkt.src_ip;
		broadcast.rs_pkt.mode = RS_LOCATION_BROADCAST;

		broadcast.distance = rDst_calc_dist(stp->x, dst_got->latitude, stp->y, dst_got->longitude);

		if (dest_switch == NULL) {
			for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) {
				if (curr->assign.rs_pkt.src_ip == pkt->src_ip) {
					dest_switch = curr;
				}
			}
		}
		dest_switch->distance = broadcast.distance;
		rDst_Bshow(&broadcast, stderr);
		memcpy(&broadcast_cpy, &broadcast, sizeof(RDstPktB_t));

		rMap_append(&stp->map, broadcast.rs_pkt.src_ip, broadcast.target_ip, broadcast.distance);
		/* BROADCAST UDP Side */
		if (stp->sup && stp->name[0] != '\0') {
			fprintf(stderr, "ERR: BROADCAST UDP (%s) SIDE to connected to just me\n", stp->sup->name);
			rsPkt_ip_op_put(&broadcast.target_ip, stp->sup->name);
			rDst_Bshow(&broadcast, stderr);
			memcpy(client->buf, &broadcast, sizeof(RDstPktB_t));
			rDstB_convert_((RDstPktB)client->buf, true);
			usleep(200);
			rTcp_write(client);
			memcpy(&broadcast_cpy, &broadcast, sizeof(RDstPktB_t));
		}

		/* BROADCAST WRITE add to record */

		/*for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) { // list
			memcpy(curr->connection->buf, &broadcast, sizeof(RDstPktB_t));
			rTcp_write(curr->connection);
		}*/
		for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) { // conn list
			memcpy(&broadcast, &broadcast_cpy, sizeof(RDstPktB_t)); // reset
			if (curr->assign.rs_pkt.src_ip == broadcast.target_ip) {
				fprintf(stderr, "ERR: con list broadcast target is assign source\n");
				continue;
			}
			if (curr->assign.rs_pkt.mode == RS_ACKNOWLEDGE) {
				broadcast.rs_pkt.dest_ip = curr->assign.rs_pkt.src_ip;
				broadcast.rs_pkt.src_ip = curr->assign.rs_pkt.dest_ip;
				broadcast.distance += curr->distance;
				rDst_Bshow(&broadcast, stderr);
				if (broadcast.rs_pkt.dest_ip == broadcast.target_ip) { continue; }
				memcpy(curr->connection->buf, &broadcast, sizeof(RDstPktB_t));
				rDstB_convert_((RDstPktB)curr->connection->buf, true);
				rTcp_write(curr->connection);
			}
		}

		for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) { // normal list
			memcpy(&broadcast, &broadcast_cpy, sizeof(RDstPktB_t)); // reset
			if (curr->assign.rs_pkt.src_ip == broadcast.target_ip) {
				fprintf(stderr, "ERR: broadcast target is assign source\n");
				continue;
			}
			rsPkt_set_src_ip((RSPkt)&broadcast, stp->tbox->tbox.host);
			rsPkt_set_dest_ip((RSPkt)&broadcast, curr->ip_name);
			broadcast.distance += curr->distance;
			rDst_Bshow(&broadcast, stderr);
			memcpy(curr->connection->buf, &broadcast, sizeof(RDstPktB_t));
			if (broadcast.rs_pkt.dest_ip == broadcast.target_ip) { continue; }
			rDstB_convert_((RDstPktB)curr->connection->buf, true);
			rTcp_write(curr->connection);
		}

		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_LOCATION_BROADCAST)) {
		RDstPktB broadcast = (RDstPktB)client->buf;
		broadcast->distance = ntohl(broadcast->distance);
		rDst_Bshow(broadcast, stderr);

		if (rMap_append(&stp->map, broadcast->rs_pkt.src_ip, broadcast->target_ip, broadcast->distance) == false) {
			return false;
		}

		/* BROADCAST WRITE if new distance is lesser than older update record */
		for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) { // normal list
			RDstPktB_t broadcast_send;
			memset(&broadcast_send, 0, sizeof(RDstPktB_t));
			uint32_t my_ip = 0, his_ip = 0;
			rsPkt_ip_op_put(&his_ip, curr->ip_name);
			if (his_ip == broadcast->target_ip || his_ip == broadcast->rs_pkt.dest_ip || his_ip == broadcast->rs_pkt.src_ip) {
				continue;
			}
			rsPkt_ip_op_put(&my_ip, stp->name);
			broadcast_send.target_ip = broadcast->target_ip;
			broadcast_send.rs_pkt.src_ip = my_ip;
			broadcast_send.rs_pkt.dest_ip = his_ip;
			broadcast_send.rs_pkt.mode = RS_LOCATION_BROADCAST;
			broadcast_send.distance = broadcast->distance + curr->distance;
			rDst_Bshow(&broadcast_send, stderr);
			memcpy(curr->connection->buf, &broadcast_send, sizeof(RDstPktB_t));
			rDstB_convert_((RDstPktB)curr->connection->buf, true);
			rTcp_write(curr->connection);
		}

		for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) { // con list
			RDstPktB_t broadcast_send;
			memset(&broadcast_send, 0, sizeof(RDstPktB_t));
			uint32_t my_ip = 0, his_ip = curr->assign.rs_pkt.src_ip;
			//rsPkt_ip_op_put(&his_ip, curr->ip_name);
			if (his_ip == broadcast->target_ip || his_ip == broadcast->rs_pkt.dest_ip || his_ip == broadcast->rs_pkt.src_ip) {
				continue;
			}
			rsPkt_ip_op_put(&my_ip, curr->ip_name);
			broadcast_send.target_ip = broadcast->target_ip;
			broadcast_send.rs_pkt.src_ip = my_ip;
			broadcast_send.rs_pkt.dest_ip = his_ip;
			broadcast_send.rs_pkt.mode = RS_LOCATION_BROADCAST;
			broadcast_send.distance = broadcast->distance + curr->distance;
			rDst_Bshow(&broadcast_send, stderr);
			memcpy(curr->connection->buf, &broadcast_send, sizeof(RDstPktB_t));
			rDstB_convert_((RDstPktB)curr->connection->buf, true);
			rTcp_write(curr->connection);
		}

		//rMap_append(&stp->map, broadcast.)

	} else if (RSPKT_MODE_CHK(pkt, RS_REQUEST)) {
		RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head, ((RGreet)pkt)->assign_ip, NULL);
		if (dest_switch == NULL) { return false; }
		RGreet greet_pkt = (RGreet)dest_switch->buf;
		greet_pkt->rs_pkt.dest_ip = greet_pkt->assign_ip;
		RSPKT_MODE_SET((RSPkt)greet_pkt, RS_ACKNOWLEDGE);
		memcpy(pkt, greet_pkt, sizeof(RGreet_t));
		fprintf(stderr, "SENDING THIS\n");
		rGreet_show((RGreet)pkt, stderr);
		// rUdp_write(sup->ubox, client);
		rTcp_write(client);
		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_QUERY_ACCEPT)) {
		rsPkt_show(pkt, stderr);
		RSwitch dest_switch = rsSvrTcp_find_switch_ip(stp->head, pkt->src_ip, NULL);
		if (dest_switch == NULL) {
			dest_switch = rsSvrTcp_find_switch_ip(stp->head_con, pkt->dest_ip, NULL);
			if (dest_switch == NULL) { return false; }
		}

		RStr curr = dest_switch->mailbox.head;
		do {
			memcpy(dest_switch->connection->buf, curr->buf_extra, sizeof(RData_t));
			rData_show((RData)dest_switch->connection->buf, stderr);
			rTcp_write_short(client); /* Check this for longer packets */
			curr = curr->next;
		} while (curr != NULL);
		RStr_destroy(dest_switch->mailbox.head, true);
		memset(&dest_switch->mailbox, 0, sizeof(RStr_lst_t));
		return true;
	} else if (RSPKT_MODE_CHK(pkt, RS_QUERY_PACKET)) {
		rsPkt_show(pkt, stderr);
		RSPkt_t snd;
		memset(&snd, 0, sizeof(RSPkt_t));
		snd.src_ip = pkt->dest_ip;
		snd.dest_ip = pkt->src_ip;
		RSPKT_MODE_SET(&snd, RS_QUERY_ACCEPT);
		rsPkt_show(&snd, stderr);
		memcpy(client->buf, &snd, sizeof(RSPkt_t));
		rTcp_write(client);
	} else if (RSPKT_MODE_CHK(pkt, RS_DATA_PAYLOAD)) {
		rsPkt_show(pkt, stderr);
		RAdaptor dest_adaptor = NULL;
		if (stp->sup != NULL) { dest_adaptor = rsSvrUdp_find_adaptor_ip(stp->sup->head, pkt->dest_ip); }
		if (dest_adaptor == NULL) {
			rData_show((RData)pkt, stderr);
			RSwitch found_sw = NULL;
			for (RSwitch curr = stp->head; curr != NULL; curr = curr->next) {
				if (curr->connection->fd == client->fd) {
					found_sw = curr;
				}
			}
			if (found_sw == NULL) {
				for (RSwitch curr = stp->head_con; curr != NULL; curr = curr->next) {
					if (curr->connection->fd == client->fd) {
						found_sw = curr;
					}
				}	
			}
			rTabel_append(&stp->tabel, found_sw, pkt->src_ip);
			rsSvrTcp_route_handle(stp, (RData)pkt, &client->excess);
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
		rsPkt_set_src_ip(&craft, stp->sup->name);
		craft.dest_ip = pkt->dest_ip;
		RSPKT_MODE_SET(&craft, RS_QUERY_PACKET);
		memcpy(client_send.buf, &craft, sizeof(RSPkt_t));
		rsPkt_show(&craft, stderr);
		rUdp_write(stp->sup->ubox, &client_send);
		rData_show(&dat_pkt, stderr);
	} else {
		fprintf(stderr, "ERR: Not implemented\n");
	}
	return false;
}

#endif //__RSRVTHANDLE__H__
