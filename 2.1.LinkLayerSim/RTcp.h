#ifndef __RTCP__H__
#define __RTCP__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include "RSockCom.h"

#include <poll.h>

#define TCP_BUF_MAX_SZ RSOCK_BUF_MAX_SZ

#define RTCP_HOST RSOCK_HOST
#define RTCP_PORT 0/*RSOCK_PORT*/

#define RTCP_BACKLOG 8

#define RTCP_MAX_POLL 1024

#define CONENCT_COMMAND "connect "
#define LOCALHOST_ADDRV	16777343

/*
 * RUdp struct: user inserts uhost
 */
typedef struct RTcp_t {
	struct RSock_t tbox;
} RTcp_t;
typedef RTcp_t *RTcp;

int
rTcp_setup(RTcp tbox)
{
	tbox->tbox.fd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if (tbox->tbox.fd < 0) {
		fprintf(stderr, \
			"socket: %d %s\n", \
			tbox->tbox.fd, strerror(errno));
		exit(EXIT_FAILURE);
	}

	tbox->tbox.ijunk = 0;
	tbox->tbox.error = \
		setsockopt(tbox->tbox.fd, SOL_SOCKET, SO_REUSEADDR, \
		&tbox->tbox.ijunk, sizeof(tbox->tbox.ijunk));
	if (tbox->tbox.error != 0) {
		fprintf(stderr, \
			"setsockopt: %d %s\n", \
			tbox->tbox.fd, strerror(errno));
		exit(EXIT_FAILURE);
	}

	int fd_oops = fcntl(tbox->tbox.fd, F_GETFL);
	if (fd_oops < 0) {
		fprintf(stderr, \
			"fcntl: F_GETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}
	tbox->tbox.error = fcntl(tbox->tbox.fd, F_SETFL, fd_oops | O_NONBLOCK);
	if (tbox->tbox.error < 0) {
		fprintf(stderr, \
			"fcntl: F_SETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}

	tbox->tbox.addr_sz = sizeof(struct sockaddr_in);
	memset(&tbox->tbox.addr, 0, sizeof(tbox->tbox.addr_sz));
	tbox->tbox.addr.sin_family = AF_INET;
	tbox->tbox.addr.sin_port = (RTCP_PORT == 0) ? 0 : htons(RTCP_PORT);
	tbox->tbox.addr.sin_addr.s_addr = \
		RTCP_HOST == NULL ? INADDR_ANY : inet_addr(RTCP_HOST);
	
	tbox->tbox.error = bind(tbox->tbox.fd, (struct sockaddr *)&tbox->tbox.addr, sizeof(struct sockaddr_in));
	if (tbox->tbox.error < 0) {
		fprintf(stderr, \
			"connect: %d %s\n", \
			tbox->tbox.error, strerror(errno));
		exit(EXIT_FAILURE);
	}

	tbox->tbox.error = listen(tbox->tbox.fd, RTCP_BACKLOG);
	if (tbox->tbox.error < 0) {
		fprintf(stderr, \
			"listen: %d %s\n", \
			tbox->tbox.error, strerror(errno));
		exit(EXIT_FAILURE);
	}

	RSCom_name(&tbox->tbox, true);

	fprintf(stdout, "%d\n", ntohs(tbox->tbox.junk.sin_port));
	fflush(stdout);
	fprintf(stderr, "Hello, RServer\n");
	fprintf(stderr, "Address={%s}, port={%d}\n", RTCP_HOST, ntohs(tbox->tbox.junk.sin_port));

	return tbox->tbox.error;
}

int
rTcp_read(RSock client)
{
	memset(client->buf, 0, TCP_BUF_MAX_SZ);
	client->addr_sz = sizeof(struct sockaddr_in);
	return client->error = recvfrom(client->fd, client->buf, TCP_BUF_MAX_SZ, 0, \
		(struct sockaddr *)&client->addr, &client->addr_sz);
}

/*
 * Use this for short write past the rspkt mark.
 */
int
rTcp_write_short(RSock client)
{
	size_t buf_sz = strnlen(client->buf + 12, TCP_BUF_MAX_SZ - 12);
	client->addr_sz = sizeof(struct sockaddr_in);
	return client->error = sendto(client->fd, client->buf, buf_sz + 12, 0, \
		(struct sockaddr *)&client->addr, client->addr_sz);
}

int
rTcp_write(RSock client)
{
	client->addr_sz = sizeof(struct sockaddr_in);
	return client->error = sendto(client->fd, client->buf, TCP_BUF_MAX_SZ, 0, \
		(struct sockaddr *)&client->addr, client->addr_sz);
}

int
rTcp_accept(RSock client, int listen)
{
	return client->fd = accept(listen, (struct sockaddr *)&client->addr, &client->addr_sz);
}

int
rTcp_check_stdin(char buf[TCP_BUF_MAX_SZ])
{
	int port = 0;
  int ret = strncmp(buf, CONENCT_COMMAND, sizeof(CONENCT_COMMAND) - 1);
  if (ret == 0) {
    ret = sscanf(buf + (sizeof(CONENCT_COMMAND) - 1), "%d", &port);
  } else {
		ret = -1;
	}
	return ret != 1 ? -1 : port;
}

bool
rTcp_peer_connect(RSock peer, int port)
{
	int fd_oops = -1;

	peer->addr_sz = sizeof(struct sockaddr_in);
	peer->addr.sin_family = AF_INET;
	peer->addr.sin_addr.s_addr = LOCALHOST_ADDRV;
	peer->addr.sin_port = htons(port);

	fprintf(stderr, "%s: connecting to port-{%d}\n", __func__, port);

  peer->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (peer->fd < 0) {
		fprintf(stderr, \
			"socket: %d %s\n", \
			peer->fd, strerror(errno));
		goto FRESH_PEER_BAD;
	}

  peer->error = connect(peer->fd, (struct sockaddr *)&peer->addr, peer->addr_sz);
	if (peer->error != 0) {
		fprintf(stderr, \
			"connect: %d %s\n", \
			peer->error, strerror(errno));
		goto FRESH_PEER_BAD;
	}
	fd_oops = fcntl(peer->fd, F_GETFL);
	if (fd_oops < 0) {
		fprintf(stderr, \
			"fcntl: F_GETFL %d %s\n", \
			fd_oops, strerror(errno));
			goto FRESH_PEER_BAD;
	}
  peer->error = fcntl(peer->fd, F_SETFL, fd_oops | O_NONBLOCK);
	if (peer->error < 0) {
		fprintf(stderr, \
			"fcntl: F_SETFL %d %s\n", \
			fd_oops, strerror(errno));
			goto FRESH_PEER_BAD;
	}
	return true;
FRESH_PEER_BAD:
	return false;
}

void
rTcp_whitetest_echo(void)
{
	RTcp_t tbox;
	RTcp tbox_p = &tbox;

	rTcp_setup(tbox_p);

	while (1) {
		RSock_t client;
		memset(&client, 0, sizeof(RSock_t));
		if (rTcp_accept(&client, tbox_p->tbox.fd) > 0 && rTcp_read(&client) > 0) {
			RSCom_name(&client, false);
			RSCom_show_name(&client, stderr);
			printf("buf: %s\n", client.buf);
			rTcp_write(&client);
			close(client.fd);
		}
	}
	return;
}

void
rTcp_whitetest(void)
{
	RTcp_t tbox;
	RTcp tbox_p = &tbox;
	int npoll = 2, cpoll = 0, ret = -1;

	struct pollfd pfds[RTCP_MAX_POLL];
	memset(pfds, 0, sizeof(pfds));

	RSock_t clients[RTCP_MAX_POLL];
	memset(&clients, 0, sizeof(clients));

	rTcp_setup(tbox_p);

	clients[0].fd = STDIN_FILENO;
	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;

	pfds[1].fd = tbox.tbox.fd;
	pfds[1].events = POLLIN;

	while (1) {
		if (cpoll = ret = poll(pfds, npoll % RTCP_MAX_POLL, -1), ret < 0) {
			fprintf(stderr, \
			"poll: %d %s\n", \
			ret, strerror(errno));
		}
		for (int ipoll = 0; ipoll < npoll && cpoll >= 0; ipoll++) {
			/* Close */
			if (pfds[ipoll].revents & (POLLHUP|POLLERR|POLLNVAL)) {
				memset(&clients[ipoll], 0, sizeof(RSock_t));
				memset(&pfds[ipoll], 0, sizeof(struct pollfd));
				pfds[ipoll].fd = ~0;
				close(clients[ipoll].fd);
				cpoll--;
			}
			if ((pfds[ipoll].revents & POLLIN)) {
				if (pfds[ipoll].fd == STDIN_FILENO) { /* Read stdin */
					clients[ipoll].error = clients[ipoll].buf_read_sz = read(clients[ipoll].fd, clients[ipoll].buf, TCP_BUF_MAX_SZ);
					clients[ipoll].buf[clients[ipoll].buf_read_sz] = '\0';
					clients[ipoll].fd = pfds[ipoll].fd = STDOUT_FILENO;
					pfds[ipoll].events = POLLOUT;
				} else if ((pfds[ipoll].fd == tbox.tbox.fd) && rTcp_accept(&clients[npoll], tbox_p->tbox.fd)) { /* Accept */
					RSCom_name(&clients[npoll], false);
					RSCom_show_name(&clients[npoll], stderr);
					pfds[npoll].fd = clients[npoll].fd;
					pfds[npoll].events = POLLIN;
					npoll++;
				} else { /* Read */
					if (rTcp_read(&clients[ipoll]) < 0) {
						switch (errno) {
							case EAGAIN:
								//nanosleep(&tv, NULL);
								continue;
								break;
							/*case EWOULDBLOCK:
								break;*/
							default:
								fprintf(stderr, \
									"recv: %d %s\n", \
									ret, strerror(errno));
								//goto CCLOSE;
								continue;
						}
						memset(&clients[ipoll], 0, sizeof(RSock_t));
						memset(&pfds[ipoll], 0, sizeof(struct pollfd));
						pfds[ipoll].fd = ~0;
						close(clients[ipoll].fd);
					} else {
						printf("ipoll={%d}, fd={%d}, buf: %s\n", ipoll, clients[ipoll].fd, clients[ipoll].buf);
						pfds[ipoll].events = POLLIN | POLLOUT;
					}
				}
				cpoll--;
			}
			/* Write */
			if (pfds[ipoll].revents & POLLOUT) {
				if (pfds[ipoll].fd == STDOUT_FILENO) { /* Write stout */
					clients[ipoll].error = write(clients[ipoll].fd, clients[ipoll].buf, TCP_BUF_MAX_SZ);
					clients[ipoll].fd = pfds[ipoll].fd = STDIN_FILENO;
					pfds[ipoll].events = POLLIN;
				} else if (rTcp_write(&clients[ipoll]) < 0) {
					switch (errno) {
						case EAGAIN:
							//nanosleep(&tv, NULL);
							continue;
							break;
						/*case EWOULDBLOCK:
							break;*/
						default:
							fprintf(stderr, \
								"recv: %d %s\n", \
								ret, strerror(errno));
							//goto CCLOSE;
							continue;
					}
					memset(&clients[ipoll], 0, sizeof(RSock_t));
					memset(&pfds[ipoll], 0, sizeof(struct pollfd));
					pfds[ipoll].fd = ~0;
					close(clients[ipoll].fd);
				} else {
					pfds[ipoll].events = POLLIN;
				}
				cpoll--;
			}
		}
	}
	return;
}

#endif //__RTCP__H__