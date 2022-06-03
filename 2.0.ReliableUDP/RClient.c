#define POSIX_C_SOURCE >= 200112L
#define _POSIX_C_SOURCE

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "RHdr.h"
#include "RQueue.h"
#include "RState.h"

#define BUF_SIZ		sizeof(RHdr_t)
#define RESOURCE	"file.0.txt"
#define PORTNUM		"5001"
#define ADDRVAL		"127.0.0.1"
#define DEFAULT_NUM	"0"
#define CLIENT_APPEND	".client."
#define MAX_FNAME 	64
#define MAX_DEAD	500U

void
check_time(time_t *freeze)
{
	time_t freeze_new = time(NULL);
	if (*freeze - freeze_new > MAX_DEAD) {
		fprintf(stderr, "ERROR: timeout\n");
		exit(EXIT_FAILURE);
	} else {
		bcopy(&freeze_new, freeze, sizeof(time_t));
	}

	return;
}

int
main(int argc, char *argv[])
{
	printf("Hello, RClient pid={%d}\n", getpid());
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int sfd, s, opt, misr, xubr = 0;
	ssize_t nread;
	RState_t rstate;
	bool chksum = false, \
		encryption = false, \
		misbehave = false, misbehaveD = false, \
		xuberant = false;
	char *address = ADDRVAL, \
		*port = PORTNUM, *number = DEFAULT_NUM, \
		*resource = RESOURCE, fname[MAX_FNAME];
	time_t freeze = time(NULL);

	while ((opt = getopt(argc, argv, "a:p:n:r:cemx:")) != -1) {
		switch (opt) {
			case 'p':
				port = optarg;
				break;
			case 'a':
				address = optarg;
				break;
			case 'n':
				number = optarg;
				break;
			case 'r':
				resource = optarg;
				break;
			case 'c':
				chksum = true;
				break;
			case 'e':
				//encryption = true;
				break;
			case 'm':
				misbehave = true;
				break;
			case 'x':
				//xuberant = true;
				//xubr = atoi(optarg);
				break;
			default:
				fprintf(stderr, \
					"Usage: %s [-a address] [-p port] [-n number] [-r resource]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	printf("Address={%s}, port={%s}\n", address, port);
	fflush(stdout);

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;		  /* Any protocol */

	s = getaddrinfo(address, port, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
		Try each address until we successfully connect(2).
		If socket(2) (or connect(2)) fails, we (close the socket
		and) try the next address. */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype,
					rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
			break;				  /* Success */

		close(sfd);
	}

	if (rp == NULL) {			   /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);		   /* No longer needed */

	/* Send remaining command-line arguments as separate
		datagrams, and read responses from server */

	uint8_t pos = 0;
	memcpy(fname + pos, number, strlen(number));
	pos += strlen(number);
	memcpy(fname + pos, CLIENT_APPEND, strlen(CLIENT_APPEND));
	pos += strlen(CLIENT_APPEND);
	memcpy(fname + pos, resource, strlen(resource));
	pos += strlen(resource);
	fname[pos] = '\0';

	FILE *req = fopen(fname, "w");

	RSTATE_INIT(rstate);

	Data buf_str[MSG_SIZ];
	memset(buf_str, 0, MSG_SIZ);
	strncpy(buf_str, resource, MSG_SIZ);

	Entry e[2];
	e[MYS] = rhead_append(&rstate.states[MYS]);
	Data *chk = entry_puts(e[MYS], buf_str);
	if (chk == NULL) {
		printf("chk=NULL\n");
	} else {
		printf("chk=%s\n", chk);
	}
	FLAG_SET(e[MYS]->hdr, GET);

	uint16_t track_seqn = 1;
	bool append_fl = true;
	while (1) {
		if (chksum == true) {
			FLAG_SET(e[MYS]->hdr, CHK);
			e[MYS]->hdr.content[CHKS] = \
				rhdr_computer_chksum(&e[MYS]->hdr);
		}
		if (encryption == true) {
			FLAG_SET(e[MYS]->hdr, ENC);
			encrypt(&e[MYS]->hdr);
		}
		printf("MYS");
//		e[MYS]->hdr.content[FLGV] &= 0xFE00;
//		e[MYS]->hdr.msg[0] = 'a';
		rhdr_pretty_print(&e[MYS]->hdr);
		convert_bytes(&e[MYS]->hdr, false);
		ssize_t ret = write(sfd, &e[MYS]->hdr, BUF_SIZ);
		if (ret < 0) {

		} else if ((size_t)ret != BUF_SIZ) {
			fprintf(stderr, "partial/failed write\n");
			exit(EXIT_FAILURE);
		}
		convert_bytes(&e[MYS]->hdr, true);

		e[HIS] = rhead_append(&rstate.states[HIS]);
		nread = read(sfd, &e[HIS]->hdr, BUF_SIZ);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}
		convert_bytes(&e[HIS]->hdr, true);
		if (encryption && FLAG_CHECK(e[HIS]->hdr, ENC) == true) {
			decrypt(&e[HIS]->hdr);
		} else if (encryption && FLAG_CHECK(e[HIS]->hdr, ENC) == false) {
			goto INVALID_NAK;
		}
		if (chksum && (valid_chksum(&e[HIS]->hdr) == false)) {
			goto INVALID_NAK;
		} else {
			FLAG_UNSET((e[HIS]->hdr), CHK);
		}
		if (FLAG_VALIDATE(e[HIS]->hdr, FIN) == false && \
			misbehave == true && track_seqn > 1) {
			misr = rand();
			misr %= 2;
			printf("rand{%d}\n", misr);
			if (misr == 1) {
				misbehaveD = true;
				goto INVALID_NAK;
			}
		}

		if (FLAG_CHECK(e[HIS]->hdr, (FIN|ACK)) == true && \
			e[HIS]->hdr.content[ACKN] == e[MYS]->hdr.content[SEQN]) {
#ifdef DEBUG
			rstate_show(&rstate);
#endif
			rstate_destroy(&rstate);
			break;
		} else if (FLAG_CHECK(e[HIS]->hdr, FIN) == true \
			&& e[HIS]->hdr.content[SEQN] == track_seqn) {
			e[MYS] = rhead_append(&rstate.states[MYS]);
			if (xuberant == true && xubr > 0) {
				xubr -= 1;
				printf("%d\n", track_seqn);
				track_seqn = 1;
				FLAG_SET(e[MYS]->hdr, DAT);
				FLAG_SET(e[MYS]->hdr, NAK);
			} else {
				FLAG_SET(e[MYS]->hdr, ACK);
				FLAG_SET(e[MYS]->hdr, FIN);
			}
			e[MYS]->hdr.content[ACKN] = track_seqn;
		} else if (FLAG_CHECK(e[HIS]->hdr, DAT) == true && \
			e[HIS]->hdr.content[SEQN] == track_seqn) {
			e[MYS] = rhead_append(&rstate.states[MYS]);
			FLAG_SET(e[MYS]->hdr, ACK);
			FLAG_SET(e[MYS]->hdr, DAT);
			e[MYS]->hdr.content[ACKN] = track_seqn;
		} else {
INVALID_NAK:
			rhdr_pretty_print(&e[HIS]->hdr);
			fprintf(stdout, "exiting failure");
			//exit(EXIT_FAILURE);
			e[MYS] = rhead_append(&rstate.states[MYS]);
			if (misbehaveD == true) {
				FLAG_SET(e[MYS]->hdr, DAT);
			}
			FLAG_SET(e[MYS]->hdr, NAK);
			e[MYS]->hdr.content[ACKN] = track_seqn;
			track_seqn -= 1;
			append_fl = false;
		}
		if (append_fl == true) {
			fprintf(req, e[HIS]->hdr.msg, MSG_SIZ);
			fflush(req);
		}
		printf("HIS");
		rhdr_pretty_print(&e[HIS]->hdr);
		track_seqn += 1;
		append_fl = true;
		check_time(&freeze);
//		e[MYS]->hdr.content[FLGV] &= 0xFE00;
//		e[MYS]->hdr.msg[0] = 'a';
	}

	fclose(req);

	exit(EXIT_SUCCESS);
}
