#ifndef __RUDP__H__
#define __RUDP__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <sys/ioctl.h>

#include "RSockCom.h"

/*sizeof(struct RSPkt_t)*/
#define RSPKT_SZ 12

#define UDP_BUF_MAX_SZ RSOCK_BUF_MAX_SZ

#define RUDP_HOST RSOCK_HOST
#define RUDP_PORT 0/*RSOCK_PORT*/

#define RUDP_EXTRA_N_MAX 1024
#define RUDP_EXTRA_BUF_MAX_SZ (RSOCK_BUF_MAX_SZ - RSPKT_SZ)

/*
 * RUdp struct: user inserts uhost
 */
typedef struct RUdp_t {
	struct RSock_t ubox;
} RUdp_t;
typedef RUdp_t *RUdp;

int
rUdp_setup(RUdp ubox)
{
	ubox->ubox.fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ubox->ubox.fd < 0) {
		fprintf(stderr, \
			"socket: %d %s\n", \
			ubox->ubox.fd, strerror(errno));
		exit(EXIT_FAILURE);
	}

  ubox->ubox.ijunk = 0;
	ubox->ubox.error = \
		setsockopt(ubox->ubox.fd, SOL_SOCKET, SO_REUSEADDR, \
		&ubox->ubox.ijunk, sizeof(ubox->ubox.ijunk));
	if (ubox->ubox.error != 0) {
		fprintf(stderr, \
			"setsockopt: %d %s\n", \
			ubox->ubox.fd, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ubox->ubox.addr_sz = sizeof(struct sockaddr_in);
	memset(&ubox->ubox.addr, 0, sizeof(ubox->ubox.addr_sz));
	ubox->ubox.addr.sin_family = AF_INET;
	ubox->ubox.addr.sin_port = (RUDP_PORT == 0) ? 0 : htons(RUDP_PORT);
	ubox->ubox.addr.sin_addr.s_addr = \
		RUDP_HOST == NULL ? INADDR_ANY : inet_addr(RUDP_HOST);
	
	ubox->ubox.error = bind(ubox->ubox.fd, (struct sockaddr *)&ubox->ubox.addr, sizeof(struct sockaddr_in));
	if (ubox->ubox.error < 0) {
		fprintf(stderr, \
			"connect: %d %s\n", \
			ubox->ubox.error, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&ubox->ubox.junk, 0, sizeof(struct sockaddr_in));
	ubox->ubox.junk_sz = sizeof(struct sockaddr_in);
	ubox->ubox.error = getsockname(ubox->ubox.fd, (struct sockaddr *)&ubox->ubox.junk, &ubox->ubox.junk_sz);
	if (ubox->ubox.error != 0) {
		fprintf(stderr, \
			"getsockname: %d %s\n", \
			ubox->ubox.error, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%d\n", ntohs(ubox->ubox.junk.sin_port));
	fflush(stdout);
	fprintf(stderr, "Hello, RServer\n");
	fprintf(stderr, "Address={%s}, port={%d}\n", RUDP_HOST, ntohs(ubox->ubox.junk.sin_port));

	int fd_oops = fcntl(ubox->ubox.fd, F_GETFL);
	if (fd_oops < 0) {
		fprintf(stderr, \
			"fcntl: F_GETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}
	ubox->ubox.error = fcntl(ubox->ubox.fd, F_SETFL, fd_oops | O_NONBLOCK);
	if (ubox->ubox.error < 0) {
		fprintf(stderr, \
			"fcntl: F_SETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}
	return ubox->ubox.error;
}

int
rUdp_read(RUdp ubox, RSock client)
{
	memset(client->buf, 0, UDP_BUF_MAX_SZ);
	client->addr_sz = sizeof(struct sockaddr_in);
	client->excess.excess_on = false;

	/* Check sz */
	int ex_sz;
	client->error = ioctl(ubox->ubox.fd, FIONREAD, &ex_sz);
	if (client->error) {
		fprintf(stderr, "ioctl: %d %s", client->error, strerror(errno));
	}
	if(/*ex_sz = recv(ubox->ubox.fd, client->buf, UDP_BUF_MAX_SZ, MSG_PEEK | MSG_TRUNC), */ex_sz > UDP_BUF_MAX_SZ) {
		client->excess.excess_on = true;
	}
	if (!client->excess.excess_on) {
		client->error = client->buf_read_sz = recvfrom(ubox->ubox.fd, client->buf, UDP_BUF_MAX_SZ, 0, \
			(struct sockaddr *)&client->addr, &client->addr_sz);
			fprintf(stderr, "recvfrom: %d %d %s\n", client->error, errno, strerror(errno));
	} else {
		fprintf(stderr, "ex_sz={%d}\n", ex_sz);
		RStr_init(&client->excess, ex_sz + 1);
		// client->excess.buf_extra = malloc(ex_sz);
		client->error = client->excess.buf_extra_sz = recvfrom(ubox->ubox.fd, client->excess.buf_extra, ex_sz, 0, \
			(struct sockaddr *)&client->addr, &client->addr_sz);
		if (client->error > 0) {
			fprintf(stderr, "extra: %s\n", client->excess.buf_extra);
		}
	}
	memcpy(&client->junk, &client->addr, client->addr_sz);
	client->junk_sz = client->addr_sz;
	return client->error;
}

int
rUdp_write(RUdp ubox, RSock client)
{
	RStr_t snd;
	memset(&snd, 0, sizeof(snd));
	if (client->excess.excess_on && client->excess.buf_extra) {
		snd.buf_extra = client->excess.buf_extra;
		snd.buf_extra_sz = client->excess.buf_extra_sz;
	} else {
		snd.buf_extra = client->buf;
		snd.buf_extra_sz = UDP_BUF_MAX_SZ;
	}
	RSCom_show_name(client, stderr);
	client->error = client->buf_read_sz = sendto(ubox->ubox.fd, snd.buf_extra, snd.buf_extra_sz, 0, \
		(struct sockaddr *)&client->addr, client->addr_sz);
	fprintf(stderr, "sendto: %d %d %s\n", client->error, errno, strerror(errno));
	if (client->excess.excess_on && client->excess.buf_extra) {
		RStr_destroy(&snd, true);
	}
	return client->error;
}

void
rUdp_whitetest(void)
{
	RUdp_t ubox;
	RUdp ubox_p = &ubox;

	rUdp_setup(ubox_p);

	while (1) {
		RSock_t client;
		memset(&client, 0, sizeof(RSock_t));
		rUdp_read(ubox_p, &client);
		if (client.error == -1) {
			switch (errno) {
				case EAGAIN:
					// nanosleep(&tv, NULL);
					continue;
					break;
				/*case EWOULDBLOCK:
					break;*/
				default:
					fprintf(stderr, \
						"recv: %d %s\n", \
						client.error, strerror(errno));
					goto CCLOSE;
			}
		}
		RSCom_show_name(&client, stderr);
		fprintf(stderr, "%s\n", client.buf);
		rUdp_write(ubox_p, &client);
	}
CCLOSE:
	return;
}

#endif //__RUDP__H__