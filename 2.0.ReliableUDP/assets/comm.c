#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

#include <signal.h>

char sample_response[] = \
	"HTTP/1.1 200 OK\r\n" \
	"Date: Mon, 27 Jul 2009 12:28:53 GMT\r\n" \
	"Server: Apache\r\n" \
	"Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\r\n" \
	"Vary: Accept-Encoding\r\n" \
	"Content-Type: application/json\r\n" \
	"\r\n";

#define BUF_SIZ	1024 * 2

#define PORTNUM	5001

int sock = ~0;

void
exit_func ( int signo )
{
    fprintf(stderr, "\n got signo %d, bye.\n\n", signo);

    close(sock);

    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	int client = ~0,
		ret = ~0;
	uint32_t count = 0;
	struct sockaddr_in ssock = {0}, \
		csock = {0};
	char buf[BUF_SIZ] = {0};
	socklen_t clen = sizeof(client);

	signal( SIGKILL, exit_func );
	signal( SIGTERM, exit_func );
	signal( SIGINT,  exit_func );

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, "socket: %d %s\n", sock, strerror(errno));
		exit(EXIT_FAILURE);
	}

	bzero(&ssock, sizeof(ssock));
	ssock.sin_family = AF_INET;
	ssock.sin_port = htons(PORTNUM);
	ssock.sin_addr.s_addr = inet_addr("0.0.0.0");

	ret = bind(sock, (struct sockaddr *)&ssock, sizeof(ssock));
	if (ret < 0) {
		fprintf(stderr, "connect: %d %s\n", ret, strerror(errno));
		exit(EXIT_FAILURE);
	}

	while (1) {
		bzero(&csock, sizeof(csock));
CRECV:
		bzero(buf, BUF_SIZ);
		ret = recvfrom(sock, buf, BUF_SIZ, 0, (struct sockaddr *)&csock, &clen);
		if (ret == -1) {
			switch (errno) {
				case EAGAIN:
					break;
				/*case EWOULDBLOCK:
					break;*/
				default:
					fprintf(stderr, "recv: %d %s\n", ret, strerror(errno));
					goto CCLOSE;
			}
		}
		printf("_____start{%u}\n", count);
		printf("%s\n", buf);
		fflush(stdout);
CSEND:
		ret = sendto(sock, sample_response, sizeof(sample_response) - 1, 0, (struct sockaddr *)&csock, clen);
		// ret = send(client, buf, strlen(buf), 0);
		if (ret == -1) {
			switch (errno) {
				case EAGAIN:
					break;
				/*case EWOULDBLOCK:
					break;*/
				default:
					fprintf(stderr, "send: %d %s\n", ret, strerror(errno));
					goto CCLOSE;
			}
		}
		printf("_____end{%u}__wroten{%d}\n", count, ret);
		fflush(stdout);
CCLOSE:
		close(client);
		count++;
	}
SCLOSE:
	close(sock);

	return (EXIT_SUCCESS);
}
