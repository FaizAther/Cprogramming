/*
 *
 * Copyright 2020 The University of Queensland
 * Author: Alex Wilson <alex@uq.edu.au>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/cdefs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <err.h>

#include "http-parser/http_parser.h"
#include "log.h"
#include "metrics.h"

#include <event.h>

const int BACKLOG = 8;
const size_t BUFLEN = 2048;

enum response_type {
	RESP_NOT_FOUND = 0,
	RESP_METRICS
};

typedef char *Chunk;

/*
 * Struct representing the message to send to client
 * header and body seperated.
 */
typedef struct _Message {
	Chunk header;
	int header_size;
	int header_sent;

	Chunk body;
	int body_size;
	int body_sent;
} _Message;
typedef _Message *Message;

/* Simples names */
typedef struct event S_EV;
typedef S_EV *P_EV;

typedef struct DES {
	P_EV ev;
	P_EV ev_sig;
} DES;
typedef DES *P_DES;

/*
 * Struct representing a request that's being processed and the connection it's
 * being received on (each connection can send only one request at present, so
 * these are the same).
 */
struct req {
	int id;				/* A serial number for this req */
	int done;			/* Is the request done? 1 = true */

	struct registry *registry;	/* The metrics registry */

	struct sockaddr_in raddr;	/* Remote peer */
	size_t pfdnum;			/* pollfd index, or 0 if not polled */
	int sock;			/* Client socket */

	struct http_parser *parser;
	enum response_type resp;	/* Response type based on URL+method */

	_Message _response;	/* Message struct */
	Message response;	/* Pointer to message */

	P_EV read;
	P_EV write;
	P_EV time;

	http_parser_settings *settings;	/* HTTP parser Settings */
};

#define BUF_SIZE 2048

typedef http_parser HP;
typedef HP *P_HP;

typedef struct req REQ;
typedef REQ *P_REQ;

typedef struct RS {
	struct registry *registry;
	http_parser_settings *settings;
} RS;
typedef RS *P_RS;

static int reqids = 0;

static int on_url(http_parser *, const char *, size_t);
static int on_header_field(http_parser *, const char *, size_t);
static int on_header_value(http_parser *, const char *, size_t);
static int on_body(http_parser *, const char *, size_t);
static int on_headers_complete(http_parser *);
static int on_message_complete(http_parser *);

static char *stats_buf = NULL;
static size_t stats_buf_sz = 0;

static __dead void
usage(const char *arg0)
{
	fprintf(stderr, "usage: %s [-f] [-l logfile] [-p port]\n", arg0);
	fprintf(stderr, "listens for prometheus http requests\n");
	exit(EXIT_USAGE);
}

extern FILE *logfile;

static void reactor_loop(int, struct registry *, http_parser_settings *);

int
main(int argc, char *argv[])
{
	const char *optstring = "p:fl:";
	uint16_t port = 27600;
	int daemon = 1;

	int lsock;
	struct sockaddr_in laddr;
	http_parser_settings settings;
	struct registry *registry;
	int c;
	unsigned long int parsed;
	char *p;
	pid_t kid;
	int error = 0;

	logfile = stdout;

	/*
	 * Set up the timezone information for localtime() et al. This is used
	 * by the tslog() routines in log.c to print timestamps in local time.
	 */
	tzset();

	/* Parse our commandline arguments. */
	while ((c = getopt(argc, argv, optstring)) != -1) {
		switch (c) {
		case 'p':
			errno = 0;
			parsed = strtoul(optarg, &p, 0);
			/*
			 * If strtoul() fails, it sets errno. If there's left
			 * over characters other than a number, though, it
			 * doesn't set errno. Check if it advanced "p" all the
			 * way to the end.
			 */
			if (errno != 0 || *p != '\0') {
				errx(EXIT_USAGE, "invalid argument for "
				    "-p: '%s'", optarg);
			}
			/* Ports have to fit in a uint16_t */
			if (parsed >= UINT16_MAX) {
				errx(EXIT_USAGE, "invalid argument for "
				    "-p: '%s' (too high)", optarg);
			}
			port = parsed;
			break;
		case 'f':
			daemon = 0;
			break;
		case 'l':
			logfile = fopen(optarg, "a");
			if (logfile == NULL)
				err(EXIT_USAGE, "open('%s')", optarg);
			break;
		default:
			usage(argv[0]);
		}
	}

	if (daemon) {
		kid = fork();
		if (kid < 0) {
			err(EXIT_ERROR, "fork");
		} else if (kid > 0) {
			/* The parent process exits immediately. */
			return (0);
		}
		umask(0);
		if (setsid() < 0)
			tserr(EXIT_ERROR, "setsid");
		chdir("/");

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	/* Initialise the collector module registry */
	registry = registry_build();

	/*
	 * Set up our "settings" struct for http_parser, which contains the
	 * callbacks that will run when the parser encounters different parts
	 * of the HTTP request.
	 */
	bzero(&settings, sizeof(settings));
	settings.on_headers_complete = on_headers_complete;
	settings.on_message_complete = on_message_complete;
	settings.on_url = on_url;
	settings.on_body = on_body;
	settings.on_header_field = on_header_field;
	settings.on_header_value = on_header_value;

	/*
	 * Ignore SIGPIPE if we get it from any of our sockets: we'll poll
	 * them for read/hup/err later and figure it out anyway.
	 */
	signal(SIGPIPE, SIG_IGN);

	/* Now open our listening socket */

	lsock = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, 0);
	if (lsock < 0)
		tserr(EXIT_SOCKERR, "socket()");

	if (setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR,
				&(int){ 1 }, sizeof(int))) {
		tserr(EXIT_SOCKERR, "setsockopt(SO_REUSEADDR)");
	}

	error = fcntl(lsock, F_SETFD,
				(fcntl(lsock, F_GETFD)|O_NONBLOCK));
	if (error == -1) {
		tserr(EXIT_SOCKERR,
				"fcntl gave error %s", strerror(errno));
	}

	bzero(&laddr, sizeof(laddr));
	laddr.sin_family = AF_INET;			/* XXX: ipv6? */
	laddr.sin_port = htons(port);

	if (bind(lsock, (struct sockaddr *)&laddr, sizeof(laddr)))
		tserr(EXIT_SOCKERR, "bind(%d)", port);

	if (listen(lsock, BACKLOG))
		tserr(EXIT_SOCKERR, "listen(%d)", port);

	tslog("listening on port %d", port);

	/* And begin handling requests! */
	reactor_loop(lsock, registry, &settings);

	registry_free(registry);

	return (0);
}

static void
free_req(P_REQ req, P_EV ev)
{
	if (ev != req->time && req->time != NULL) {
		event_del(req->time);
		free(req->time);
		req->time = NULL;
	}
	tslog("freeing %d", req->id);
	free(req->parser);
	if (req->response->header != NULL)
		free(req->response->header);
	if (req->response->body != NULL)
		free(req->response->body);
	if (ev != NULL) {
		event_del(ev);
		free(ev);
	}
	close(req->sock);
	free(req);
}

static void
event_free(int cfd, short revents, void *conn) {
	P_REQ req = (P_REQ)conn;
	event_del(req->read);
	free(req->read);
	req->read = NULL;
	free_req(req, req->time);
}

static void
event_write(int cfd, short revents, void *conn) {
	P_REQ req = (P_REQ)conn;
	int sendd = 0;

	if (req->time != NULL) {
		event_del(req->time);
		free(req->time);
		req->time = NULL;
	}

	if (req->response->header_size -
			req->response->header_sent > 0) {
		sendd = send(cfd,
			(req->response->header) +
			req->response->header_sent,
			req->response->header_size -
			req->response->header_sent,
			MSG_DONTWAIT);
		req->response->header_sent += sendd;
		return;
	} else if (req->response->body_size -
			req->response->body_sent > 0) {
		sendd = send(cfd,
			(req->response->body) +
			req->response->body_sent,
			req->response->body_size -
			req->response->body_sent,
			MSG_DONTWAIT);
		req->response->body_sent += sendd;
		return;
	}
	if (req->done == 1 &&
			req->response->body_size -
			req->response->body_sent +
			req->response->header_size -
			req->response->header_sent <= 0) {
		free_req(req, req->write);
	}
}

static void
event_read(int cfd, short revents, void *conn)
{
	P_REQ req = (P_REQ)conn;
	char buf[BUF_SIZE] = {0};
	int recvd = 0,
		destroy= 0,
		plen = 0;
	bzero(buf, BUF_SIZE);

	recvd = recv(cfd, buf, BUF_SIZE, MSG_DONTWAIT);
	if (errno == EAGAIN && strlen(buf) == 0 && recvd != 0) {
		return;
	} else if (recvd == -1 && errno != EAGAIN) {
		tslog("error recv:%d: %s",
				recvd, strerror(errno));
		destroy = 1;
	} else {
		plen = http_parser_execute(req->parser, \
				req->settings, buf, recvd);

		if (req->parser->upgrade) {
			/* we don't use this, so just close */
			tslog("upgrade? %d", req->id);
			destroy = 1;
		} else if (plen != recvd) {
			tslog("http-parser gave error on %d, "
				"close", req->id);
			destroy = 1;
		}
	}
	if (req->done == 1 || destroy == 1) {
		event_del(req->read);
		free(req->read);
		req->read = NULL;
		req->write = (P_EV)malloc(sizeof(*(req->write)));
		bzero(req->write, sizeof(*(req->write)));
		event_set(req->write, cfd, EV_WRITE|EV_PERSIST,
				event_write, (void *)req);
		event_add(req->write, NULL);
		req->done = 1;
	}
}

static void
event_accept(int sfd, short revents, void *conn)
{
	P_RS reg_set = (P_RS)conn;
	struct sockaddr_in raddr = {0};
	P_HP parser = NULL;
	P_REQ req = NULL;
	int cfd = -1,
		raddr_len = sizeof(raddr),
		error = 0;

	cfd = accept4(sfd,
			(struct sockaddr *)&raddr, &raddr_len, SOCK_NONBLOCK);
	if (cfd < 0) {
		switch (errno) {
		case ECONNABORTED:
		case ECONNRESET:
			tslog("failed to accept connection "
				"from %s: %d (%s)",
				inet_ntoa(raddr.sin_addr),
				errno, strerror(errno));
			break;
		default:
			tserr(EXIT_SOCKERR, "accept()");
		}
	}

	tslog("accepted connection from %s (req %d)",
		inet_ntoa(raddr.sin_addr), reqids++);

	error = fcntl(cfd, F_SETFD,
				(fcntl(cfd, F_GETFD)|O_NONBLOCK));
	if (error == -1) {
		tserr(EXIT_ERROR, "fcntl: %s", strerror(errno));
	}

	req = (P_REQ)malloc(sizeof(*req));
	if (req == NULL) {
		tserr(EXIT_MEMORY, "malloc(%zd)",
				sizeof(struct req));
	}
	bzero(req, sizeof(*req));

	req->response = &(req->_response);
	req->response->header = malloc(sizeof(char) * BUF_SIZE);
	if (req->response->header == NULL) {
		tserr(EXIT_MEMORY, "malloc(%zd)",
				sizeof(struct req));
	}
	bzero(req->response->header, sizeof(char) * BUF_SIZE);

	parser = (P_HP)malloc(sizeof(*parser));
	if (parser == NULL) {
		tserr(EXIT_MEMORY, "malloc(%zd)",
				sizeof(struct req));
	}
	bzero(parser, sizeof(*parser));

	http_parser_init(parser, HTTP_REQUEST);
	parser->data = req;

	req->id = reqids;
	req->sock = cfd;
	req->raddr = raddr;
	req->registry = reg_set->registry;
	req->settings = reg_set->settings;
	req->parser = parser;

	req->read = (P_EV)malloc(sizeof(*(req->read)));
	if (req->read == NULL) {
		tserr(EXIT_MEMORY, "malloc: %s", strerror(errno));
	}
	bzero(req->read, sizeof(*(req->read)));

	event_set(req->read, cfd, EV_READ|EV_PERSIST,
				event_read, (void *)req);
	event_add(req->read, NULL);

	struct timeval sec_30 = { 30, 0 };

	req->time = (P_EV)malloc(sizeof(*(req->time)));
	if (req->time == NULL) {
		tserr(EXIT_MEMORY, "malloc: %s", strerror(errno));
	}
	bzero(req->time, sizeof(*(req->time)));

	event_set(req->time, cfd, EV_TIMEOUT,
				event_free, (void *)req);
	event_add(req->time, &sec_30);
}

static void
reactor_destroy(int sfd, short signal, void *conn)
{
	P_DES des = (P_DES)conn;
	struct timeval sec_30 = { 30, 0 };

	tslog("Caught signal %d exiting in 30 seconds...", signal);
	close(sfd);
	event_del(des->ev);
	free(des->ev);
	event_del(des->ev_sig);
	free(des->ev_sig);
	event_loopexit(&sec_30);
}

/*
 * Main loop.
 * Set up event and signal on socket.
 */
static void
reactor_loop(int lsock, struct registry *registry,
	http_parser_settings *settings)
{
	P_EV ev = (P_EV)malloc(sizeof(*ev));
	if (ev == NULL) {
		tserr(EXIT_MEMORY, "malloc: %s", strerror(errno));
	}
	bzero(ev, sizeof(*ev));

	P_EV ev_sig = (P_EV)malloc(sizeof(*ev_sig));
	if (ev_sig == NULL) {
		tserr(EXIT_MEMORY, "malloc: %s", strerror(errno));
	}
	bzero(ev_sig, sizeof(*ev_sig));

	event_init();

	event_set(ev, lsock, EV_READ|EV_PERSIST,
			event_accept, (void *)&(RS){registry, settings});

	event_add(ev, NULL);

	signal_set(ev_sig, SIGINT,
			reactor_destroy, ((void *)&(DES){ ev, ev_sig }));

	signal_add(ev_sig, NULL);

	event_dispatch();

	return;
}

static int
on_url(http_parser *parser, const char *url, size_t ulen)
{
	struct req *req = parser->data;
	if (parser->method == HTTP_GET &&
                    ulen >= strlen("/metrics") &&
                    strncmp(url, "/metrics", strlen("/metrics")) == 0) {
		req->resp = RESP_METRICS;
	}
	if (parser->method == HTTP_GET &&
		ulen >= strlen("/stopme") &&
		strncmp(url, "/stopme", strlen("/stopme")) == 0) {
		exit(0);
	}
	return (0);
}

static int
on_body(http_parser *parser, const char *data, size_t len)
{
	//struct req *req = parser->data;
	return (0);
}

static int
on_header_field(http_parser *parser, const char *hdrname, size_t hlen)
{
	//struct req *req = parser->data;
	return (0);
}

static int
on_header_value(http_parser *parser, const char *hdrval, size_t vlen)
{
	//struct req *req = parser->data;
	return (0);
}

static int
on_headers_complete(http_parser *parser)
{
	//struct req *req = parser->data;
	return (0);
}

static void
send_err(http_parser *parser, enum http_status status)
{
	struct req *req = parser->data;
	int printd = req->response->header_size;

	printd += sprintf(req->response->header+printd,
			"HTTP/%d.%d %d %s\r\n", parser->http_major,
			parser->http_minor, status, http_status_str(status));
	printd += sprintf(req->response->header+printd,
			"Server: obsd-prom-exporter\r\n");
	printd += sprintf(req->response->header+printd,
			"Connection: close\r\n");
	printd += sprintf(req->response->header+printd, "\r\n");

	req->response->header_size = printd;
	req->done = 1;
}

static int
on_message_complete(http_parser *parser)
{
	struct req *req = parser->data;
	FILE *mf;
	off_t off;
	int r,
		printd = 0;

	/* If we didn't recognise the method+URL, return a 404. */
	if (req->resp == RESP_NOT_FOUND) {
		send_err(parser, 404);
		return (0);
	}

	if (stats_buf == NULL) {
		stats_buf_sz = 256*1024;
		stats_buf = malloc(stats_buf_sz);
	}
	if (stats_buf == NULL) {
		tslog("failed to allocate metrics buffer");
		send_err(parser, 500);
		return (0);
	}
	mf = fmemopen(stats_buf, stats_buf_sz, "w");
	if (mf == NULL) {
		tslog("fmemopen failed: %s", strerror(errno));
		send_err(parser, 500);
		return (0);
	}

	tslog("generating metrics for req %d...", req->id);
	r = registry_collect(req->registry);
	if (r != 0) {
		tslog("metric collection failed: %s", strerror(r));
		send_err(parser, 500);
		return (0);
	}
	print_registry(mf, req->registry);
	fflush(mf);
	off = ftell(mf);
	if (off < 0) {
		send_err(parser, 500);
		return (0);
	}
	fclose(mf);
	tslog("%d done, sending %lld bytes", req->id, off);

	printd = req->response->header_size;
	printd += sprintf(req->response->header+printd,
			"HTTP/%d.%d %d %s\r\n", parser->http_major,
			parser->http_minor, 200, http_status_str(200));
	printd += sprintf(req->response->header+printd,
			"Server: obsd-prom-exporter\r\n");
	/*
	 * Prometheus expects this version=0.0.4 in the content-type to
	 * indicate that it's the prometheus text format.
	 */
	printd += sprintf(req->response->header+printd,
			"Content-Type: "
			"text/plain; version=0.0.4; charset=utf-8\r\n");
	printd += sprintf(req->response->header+printd,
			"Content-Length: %lld\r\n", off);
	printd += sprintf(req->response->header+printd,
			"Connection: close\r\n");
	printd += sprintf(req->response->header+printd,
			"\r\n");

	req->response->header_size = printd;
	printd = 0;

	req->response->body = malloc(sizeof(char) * off);
	if (req->response->body == NULL) {
		tserr(EXIT_MEMORY, "malloc %s",
				strerror(errno));
	}
	printd += sprintf(req->response->body, "%s", stats_buf);
	req->response->body_size = printd;

	req->done = 1;

	return (0);
}
