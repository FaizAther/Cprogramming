#ifndef __RSOCKCOM__H__
#define __RSOCKCOM__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "RStr.h"

#define IP_MAX_NAME 16

#define RSOCK_BUF_MAX_SZ 1500

#define RSOCK_PORT 5001
#define RSOCK_HOST "127.0.0.1"

typedef struct RSock_t {
	int fd;
	struct sockaddr_in addr, junk;
	socklen_t addr_sz, junk_sz;
	char buf[RSOCK_BUF_MAX_SZ];
	char host[IP_MAX_NAME];
	int error, ijunk, buf_read_sz;
	RStr_t excess;
} RSock_t;
typedef RSock_t *RSock;

int
RSCom_name(RSock s, bool host)
{
	memset(&s->junk, 0, sizeof(struct sockaddr_in));
	s->junk_sz = sizeof(struct sockaddr_in);
	if (host) {
		s->error = getsockname(s->fd, (struct sockaddr *)&s->junk, &s->junk_sz);
	} else {
		s->error = getpeername(s->fd, (struct sockaddr *)&s->junk, &s->junk_sz);
	}
	if (s->error != 0) {
		fprintf(stderr, \
			"get(sock|peer)name: %d %s\n", \
			s->error, strerror(errno));
		exit(EXIT_FAILURE);
	}
	return s->error;
}

int
RSCom_show_name(RSock s, FILE *stream)
{
	char buf[IP_MAX_NAME];
	inet_ntop(AF_INET, &s->junk.sin_addr, buf, IP_MAX_NAME/*sizeof(struct sockaddr_in)*/);
	if (stream != NULL) {
		s->error = fprintf(stream, "Address={%s}, port={%d}\n", buf, ntohs(s->junk.sin_port));
	}
	return s->error;
}

static void
RSCom_clone(RSock s, struct sockaddr_in *addr)
{
	memcpy(&s->addr, addr, sizeof(struct sockaddr_in));
	memcpy(&s->junk, addr, sizeof(struct sockaddr_in));
	s->addr_sz = sizeof(struct sockaddr_in);
	s->junk_sz = sizeof(struct sockaddr_in);
}

#endif //__RSOCKCOM__H__