/*
 * ather
 * The University of Queensland, 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "RHdr.h"
#include "RQueue.h"
#include "RState.h"
#include "RHandleServ.h"

extern char *optarg;

#define BUF_SIZ	RHDR_SIZ

#define PORTNUM	0U

#define CHECK_USAGE	10U

#define INTERVAL	2500000L

void
exit_func (int signo)
{
	fprintf(stderr, \
		"\nGot signo{%d}\nTerminating...\n\n", \
		signo);

	rhandle_destroy();

	close(sock);

	fprintf(stderr, "SLEEP: %u seconds, check memusage\n", CHECK_USAGE);

	sleep(CHECK_USAGE);

	pthread_exit(0);
	exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{
	int opt = ~0, fd_oops = ~0,
		ret = ~0, port = PORTNUM;
	const int on = 1;
	uint32_t count = 0;
	struct sockaddr_in ssock = {0}, \
		csock = {0}, sjunk = {0};
	char buf[BUF_SIZ] = {0};
	socklen_t clen = sizeof(struct sockaddr_in), \
		sjlen = sizeof(struct sockaddr_in);
	char *address = "127.0.0.1";
	const struct timespec tv = \
		{.tv_nsec = INTERVAL, .tv_sec = 0};
	// Initialize the empty buf defined in RProcess.h
	memset(EMPTY_BUF, 0, MSG_SIZ);

	while ((opt = getopt(argc, argv, "a:p:")) != -1) {
		switch (opt) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'a':
				address = optarg;
				break;
			default:
				fprintf(stderr, \
					"Usage: %s [-a address] [-p port]\n", \
					argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	signal(SIGKILL, exit_func);
	signal(SIGTERM, exit_func);
	signal(SIGINT,  exit_func);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, \
			"socket: %d %s\n", \
			sock, strerror(errno));
		exit(EXIT_FAILURE);
	}

	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if (ret != 0) {
		fprintf(stderr, \
			"setsockopt: %d %s\n", \
			sock, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&ssock, 0, sizeof(ssock));
	ssock.sin_family = AF_INET;
	ssock.sin_port = (port == 0) ? 0 : htons(port);
	ssock.sin_addr.s_addr = address == NULL ? INADDR_ANY : inet_addr(address) ;

	ret = bind(sock, (struct sockaddr *)&ssock, sizeof(ssock));
	if (ret < 0) {
		fprintf(stderr, \
			"connect: %d %s\n", \
			ret, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(&sjunk, 0, sizeof(sjunk));
	ret = getsockname(sock, (struct sockaddr *)&sjunk, &sjlen);
	if (ret != 0) {
		fprintf(stderr, \
			"getsockname: %d %s\n", \
			ret, strerror(errno));
		exit(EXIT_FAILURE);
	}

	fprintf(stdout, "%d\n", ntohs(sjunk.sin_port));
	fflush(stdout);
	printf("Hello, RServer\n");
	printf("Address={%s}, port={%d}\n", address, ntohs(sjunk.sin_port));

	fd_oops = fcntl(sock, F_GETFL);
	if (fd_oops < 0) {
		fprintf(stderr, \
			"fcntl: F_GETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}
	ret = fcntl(sock, F_SETFL, fd_oops | O_NONBLOCK);
	if (ret < 0) {
		fprintf(stderr, \
			"fcntl: F_SETFL %d %s\n", \
			fd_oops, strerror(errno));
			exit(EXIT_FAILURE);
	}

	while(1) {
#ifdef DEBUG_SHOW
		print_tree_root_addr(addr_tree_p);
#endif
		rhandle_retransmit(NULL);

		ret = recvfrom(sock, buf, BUF_SIZ, 0, \
			(struct sockaddr *)&csock, &clen);
		if (ret == -1) {
			switch (errno) {
				case EAGAIN:
					nanosleep(&tv, NULL);
					continue;
					break;
				/*case EWOULDBLOCK:
					break;*/
				default:
					fprintf(stderr, \
						"recv: %d %s\n", \
						ret, strerror(errno));
					goto CCLOSE;
			}
		}
		if (rhandle_serv(buf, ret, &csock, clen) == false) {
			goto CCLOSE;
		}
CCLOSE:
		count++;
	}
	close(sock);

	return (EXIT_FAILURE);
}
