#ifndef __RTUP__H__
#define __RTUP__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSPkt.h"

#include "RUdp.h"
#include "RSrvUdp.h"
#include "RSrvUHandle.h"

#include "RTcp.h"
#include "RSrvTcp.h"
#include "RSrvTHandle.h"

enum RTUP_POLL_VALS {
	RTUP_POLL_STD = 0,
	RTUP_POLL_UDP,
	RTUP_POLL_TCP,
};

typedef struct rTUP_args_t {
	bool global_on;
	char *global;
	int global_subnet;
	bool local_on;
	char *local;
	int local_subnet;
	int pos_x;
	int pos_y;
} rTUP_args_t;
typedef rTUP_args_t *rTUP_args;

typedef struct RTUp_t {
	RTcp_t tbox;
	RUdp_t ubox;
	struct pollfd pfds[RTCP_MAX_POLL];
	RSock_t clients[RTCP_MAX_POLL];
	int npoll, cpoll, ret;
	RSrvUdp_t sudp;
	RSrvTcp_t stcp;
	rTUP_args args;
} RTU_t;
typedef RTU_t *RTUp;

static void
rTup_set_(RTUp tup, int index, int fd, int events, bool increment)
{
	struct pollfd *pfds = tup->pfds;
	pfds[index].fd = fd;
	pfds[index].events = events;
	if (increment) { tup->npoll++; }
}

static RTUp
rTup_std_setup(rTUP_args state)
{
	RTUp tup = (RTUp)malloc(sizeof(*tup));
	memset(tup, 0, sizeof(*tup));

	tup->args = state;

	tup->clients[0].fd = STDIN_FILENO;

	rTup_set_(tup, RTUP_POLL_STD, STDIN_FILENO, POLLIN, true);

	if (tup->args->local_on) {
		RUdp ubox_p = &tup->ubox;
		rUdp_setup(ubox_p);
		rTup_set_(tup, RTUP_POLL_UDP, tup->ubox.ubox.fd, POLLIN, true);
		rSvrUdp_init(&tup->sudp, tup->args->local, tup->args->local_subnet);
		tup->sudp.ubox = &tup->ubox;
		rIpop_op_to_str((RIPsingle)tup->sudp.ip_op.ip_box, tup->sudp.ubox->ubox.host);
		tup->sudp.stp = &tup->stcp;
		tup->stcp.sup = &tup->sudp;
	} else {
		rTup_set_(tup, RTUP_POLL_UDP, -1, 0, true);
	}
	if (tup->args->global_on) {
		RTcp tbox_p = &tup->tbox;
		rTcp_setup(tbox_p);
		rTup_set_(tup, RTUP_POLL_TCP, tup->tbox.tbox.fd, POLLIN, true);
		rSvrTcp_init(&tup->stcp, tup->args->global, tup->args->global_subnet);
		tup->stcp.tbox = &tup->tbox;
		rIpop_op_to_str((RIPsingle)tup->stcp.ip_op.ip_box, tup->stcp.tbox->tbox.host);
		if (tup->args->local_on) { tup->stcp.sup = &tup->sudp; }
	} else {
		rTup_set_(tup, RTUP_POLL_TCP, -1, 0, true);
	}
	tup->stcp.x = tup->args->pos_x;
	tup->stcp.y = tup->args->pos_y;

	if (tup->args->local_on && tup->args->global_on) { rTup_set_(tup, RTUP_POLL_STD, -1, 0, false); }

	fprintf(stdout, "> ");
	fflush(stdout);

	return tup;
}

static void
rTup_handle_poll_close(RTUp tup, int ipoll)
{
	close(tup->clients[ipoll].fd);
	fprintf(stderr, "rtup_closed: %d, ", tup->clients[ipoll].fd);
	RSCom_show_name(&tup->clients[ipoll], stderr);
	memset(&tup->pfds[ipoll], 0, sizeof(struct pollfd));
	memset(&tup->clients[ipoll], 0, sizeof(RSock_t));
	tup->pfds[ipoll].fd = ~0;
	tup->cpoll--;
}

static void
rTup_handle_poll_read_udp(RTUp tup, int ipoll)
{
	rUdp_read(&tup->ubox, &tup->clients[ipoll]);
	if (tup->clients[ipoll].error == -1) {
		switch (errno) {
			case EAGAIN:
				// nanosleep(&tv, NULL);
				// continue;
				break;
			/*case EWOULDBLOCK:
				break;*/
			default:
				fprintf(stderr, \
					"recv: %d %s\n", \
					tup->clients[ipoll].error, strerror(errno));
				//goto CCLOSE;
		}
	}
	RSCom_show_name(&tup->clients[ipoll], stderr);
	if ((size_t)tup->clients[ipoll].buf_read_sz >= sizeof(RSPkt_t)) {
		RSPkt rspkt = (RSPkt) tup->clients[ipoll].buf;
		rsPkt_show(rspkt, stderr);
		rsSvrUdp_handle(&tup->sudp, &tup->clients[ipoll]);
	}
	fprintf(stderr, "%s\n", tup->clients[ipoll].buf);
}

static void
rTup_handle_poll_read(RTUp tup, int ipoll)
{
	if (tup->pfds[ipoll].fd == STDIN_FILENO) { /* Read stdin */
		//tup->clients[ipoll].error = tup->clients[ipoll].buf_read_sz = read(tup->clients[ipoll].fd, tup->clients[ipoll].buf, TCP_BUF_MAX_SZ);
		//tup->clients[ipoll].buf[tup->clients[ipoll].buf_read_sz] = '\0';
		char *ret = fgets(tup->clients[ipoll].buf, TCP_BUF_MAX_SZ, stdin);
		tup->clients[ipoll].error = tup->clients[ipoll].buf_read_sz = strlen(tup->clients[ipoll].buf);
		if (ret == NULL) {
			rTup_set_(tup, RTUP_POLL_STD, -1, 0, false);
		} else {
			fprintf(stdout, "> ");
			fflush(stdout);
			fprintf(stderr, "STDIN={%s}\n", tup->clients[ipoll].buf);
			int port = rTcp_check_stdin(tup->clients[ipoll].buf);
			if (port > 0 && rTcp_peer_connect(&tup->clients[tup->npoll], port) && rsSvrTcp_establish(&tup->stcp, &tup->clients[tup->npoll])) {
				/* TODO: Switch off reading stdin until peer responds with offer */
				rTup_set_(tup, RTUP_POLL_STD, -1, 0, false);
				tup->pfds[tup->npoll].fd = tup->clients[tup->npoll].fd;
				tup->pfds[tup->npoll].events = POLLIN;
				tup->npoll++;
			} else {
				memset(&tup->clients, 0, sizeof(RSock_t));
			}
		}
		//tup->clients[ipoll].fd = tup->pfds[ipoll].fd = STDOUT_FILENO;
		//tup->pfds[ipoll].events = POLLOUT;
	} else if (/*tup->args->local_on && */(tup->pfds[ipoll].fd == tup->ubox.ubox.fd)) { /* Read UDP */
		rTup_handle_poll_read_udp(tup, ipoll);
		tup->pfds[ipoll].events = POLLIN;
	} else if (/*tup->args->global_on && */(tup->pfds[ipoll].fd == tup->tbox.tbox.fd) && rTcp_accept(&tup->clients[tup->npoll], tup->tbox.tbox.fd)) { /* Accept TCP */
		RSCom_name(&tup->clients[tup->npoll], false);
		RSCom_show_name(&tup->clients[tup->npoll], stderr);
		tup->pfds[tup->npoll].fd = tup->clients[tup->npoll].fd;
		tup->pfds[tup->npoll].events = POLLIN;
		tup->npoll++;
	} else/* if (tup->args->global_on)*/ { /* Read TCP */
		if (rTcp_read(&tup->clients[ipoll]) <= 0) {
			switch (errno) {
				case EAGAIN:
					//nanosleep(&tv, NULL);
					// continue;
					break;
				/*case EWOULDBLOCK:
					break;*/
				default:
					fprintf(stderr, \
						"recv: %d %s\n", \
						tup->ret, strerror(errno));
					//goto CCLOSE;
					// continue;
			}
			close(tup->clients[ipoll].fd);
			memset(&tup->pfds[ipoll], 0, sizeof(struct pollfd));
			tup->pfds[ipoll].fd = -1;
			// memset(&tup->clients[ipoll], 0, sizeof(RSock_t)); /* got to keep this else tcp handler will seg fault */
		} else {
			bool offer = RSPKT_MODE_CHK((RSPkt)tup->clients[ipoll].buf, RS_LOCATION_SEND); /* TODO: change to distance and remove sleep */
			fprintf(stderr, "ipoll={%d}, fd={%d}, buf: %s\n", ipoll, tup->clients[ipoll].fd, tup->clients[ipoll].buf);
			rsSvrTcp_handle(&tup->stcp, &tup->clients[ipoll]);
			tup->pfds[ipoll].events = POLLIN;
			if (/*tup->pfds[RTUP_POLL_STD].fd != -1 && */offer) {
				fprintf(stderr, "Got an acknowledgement, swithcing on stdin\n");
				usleep(500); /* Python is lazy */
				rTup_set_(tup, RTUP_POLL_STD, STDIN_FILENO, POLLIN, false);
			}
		}
	}
	tup->cpoll--;
}

static void
rTup_handle_poll_write(RTUp tup, int ipoll)
{
	if (tup->pfds[ipoll].fd == tup->ubox.ubox.fd) { /* Write UDP */
		rUdp_write(&tup->ubox, &tup->clients[ipoll]);
		tup->pfds[ipoll].events = POLLIN;
	} else if (tup->pfds[ipoll].fd == STDOUT_FILENO) { /* Write stout */
		tup->clients[ipoll].error = write(tup->clients[ipoll].fd, tup->clients[ipoll].buf, tup->clients[ipoll].buf_read_sz);
		tup->clients[ipoll].fd = tup->pfds[ipoll].fd = STDIN_FILENO;
		tup->pfds[ipoll].events = POLLIN;
	} else if (rTcp_write(&tup->clients[ipoll]) < 0) { /* Write TCP */
		switch (errno) {
			case EAGAIN:
				//nanosleep(&tv, NULL);
				// continue;
				break;
			/*case EWOULDBLOCK:
				break;*/
			default:
				fprintf(stderr, \
					"recv: %d %s\n", \
					tup->ret, strerror(errno));
				//goto CCLOSE;
				// continue;
		}
		memset(&tup->clients[ipoll], 0, sizeof(RSock_t));
		memset(&tup->pfds[ipoll], 0, sizeof(struct pollfd));
		close(tup->clients[ipoll].fd);
		tup->pfds[ipoll].fd = -1;
	} else {
		tup->pfds[ipoll].events = POLLIN;
	}
	tup->cpoll--;
}

void
rTUp_arg_check(int argc, char *argv[], rTUP_args state)
{
	fprintf(stderr, "arg-count=%d\n", argc);
	if (argc != 5 && argc != 6) {
		goto ARGS_KILL;
	}
	memset(state, 0, sizeof(rTUP_args_t));
	if (strcmp(argv[1], "local") == 0) {
		state->local_on = true;
		if (argc == 6) {
			state->global_on = true;
		}
	} else if (strcmp(argv[1], "global") == 0) {
		state->global_on = true;
		if (argc != 5) {
			goto ARGS_KILL;
		}
	} else {
		goto ARGS_KILL;
	}

	int subnet = -1;
	for (int i = 0; argv[2][i] != '\0'; i++) {
		if (argv[2][i] == '/') {
			argv[2][i] = '\0';
			subnet = atoi(argv[2] + i + 1);
		}
	}
	if (subnet == -1) { goto ARGS_KILL; }
	if (state->local_on) {
		state->local = argv[2];
		state->local_subnet = subnet;
	} else {
		state->global = argv[2];
		state->global_subnet = subnet;
	}
	if (state->global_on && state->local_on) {
		state->global_subnet = -1;
		for (int i = 0; argv[3][i] != '\0'; i++) {
			if (argv[3][i] == '/') {
				argv[3][i] = '\0';
				state->global_subnet = atoi(argv[3] + i + 1);
			}
		}
		if (state->global_subnet == -1) { goto ARGS_KILL; }
		state->global = argv[3];
		state->pos_x = atoi(argv[4]);
		state->pos_y = atoi(argv[5]);
	} else {
		state->pos_x = atoi(argv[3]);
		state->pos_y = atoi(argv[4]);
	}

	return;
ARGS_KILL:
	fprintf(stderr, "ERR: BAD ARGUMENTS\n");
	exit(1);
}

void
rTUp_whitetest(int argc, char *argv[])
{
	rTUP_args_t state;
	rTUp_arg_check(argc, argv, &state);

	RTUp tup = rTup_std_setup(&state);

	while (1) {
		if (tup->cpoll = tup->ret = poll(tup->pfds, tup->npoll % RTCP_MAX_POLL, -1), tup->ret < 0) {
			fprintf(stderr, \
			"poll: %d %s\n", \
			tup->ret, strerror(errno));
		}
		for (int ipoll = 0; ipoll < tup->npoll && tup->cpoll >= 0; ipoll++) {
			/* Close */
			/*if (tup->pfds[ipoll].revents & (POLLHUP|POLLERR|POLLNVAL)) { rTup_handle_poll_close(tup, ipoll); }*/
			/* Read */
			if ((tup->pfds[ipoll].revents & POLLIN)) { rTup_handle_poll_read(tup, ipoll); }
			/* Write */
			if (tup->pfds[ipoll].revents & POLLOUT) {  rTup_handle_poll_write(tup, ipoll); }
		}
	}
	return;
}

#endif //__RTUP__H__