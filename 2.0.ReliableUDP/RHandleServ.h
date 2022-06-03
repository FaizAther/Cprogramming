#ifndef __RHANDLESERV__H__
#define __RHANDLESERV__H__

#include <bsd/sys/tree.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#include "RHdr.h"
#include "RQueue.h"
#include "RState.h"
#include "RProcess.h"

#define LOCALHOST_ADDRV	16777343
#define MAX_TIMEOUT 	4U
#define DEAD_TIMEOUT	1000U

#define RMSG_GET_ADDR(rmsg) \
	rmsg.csock.sin_addr.s_addr

#define ACTIVE_N 2
#define REQPR_N 2

#define MINI_BUF 26

enum activation {
	ONA = 0,
	CLR = 1
};

enum n_req_process {
	MSG_PR = 0,
	BAD_PR = 1
};

typedef struct RMsg_t {
	Data msg[RHDR_SIZ];
	size_t written;
	size_t left;
	in_port_t port;
	struct sockaddr csock;
	socklen_t clen;
	time_t update_stamp[2];
	RState_t state;
	pthread_t stick;
	pthread_mutex_t lock;
	bool active[ACTIVE_N];
	uint32_t nreqs[REQPR_N];
	RB_ENTRY(RMsg_t) entry;
} RMsg_t;
typedef RMsg_t *RMsg_p;

int
rmsg_cmp_port(RMsg_p n1, RMsg_p n2)
{
	if (n1 == NULL) return 1;
	if (n2 == NULL) return -1;
	if (n1->port == n2->port) return 0;
	return (n1->port < n2->port) ? \
		-1 : \
		(n1->port > n2->port);
}

typedef RB_HEAD(rhandletree_port, RMsg_t) rhandle_port_t;
typedef rhandle_port_t *rhandle_port_p;
RB_GENERATE(rhandletree_port, RMsg_t, entry, rmsg_cmp_port)

typedef struct RAddr_t {
	in_addr_t addr;
	uint32_t ncons;
	rhandle_port_t port_tree;
	time_t update_stamp;
	RB_ENTRY(RAddr_t) entry;
} RAddr_t;
typedef RAddr_t *RAddr_p;

int
rmsg_cmp_addr(RAddr_p n1, RAddr_p n2)
{
	if (n1 == NULL) return 1;
	if (n2 == NULL) return -1;
	if (n1->addr == n2->addr) return 0;
	return (n1->addr < n2->addr) ? \
		-1 : \
		(n1->addr > n2->addr);
}

typedef RB_HEAD(rhandletree_addr, RAddr_t) rhandle_tree_t;
typedef rhandle_tree_t *rhandle_tree_p;

rhandle_tree_t addr_tree_t = RB_INITIALIZER(addr_tree_t);
rhandle_tree_p addr_tree_p = &addr_tree_t;
RB_GENERATE(rhandletree_addr, RAddr_t, entry, rmsg_cmp_addr)

int sock = ~0;

void
print_node(RMsg_p n)
{
	struct sockaddr_in *csock = \
		(struct sockaddr_in *)&n->csock;
	uint16_t port = htons(csock->sin_port);;
	struct tm *tm_info = localtime(&n->update_stamp[HIS]);
	char buf[MINI_BUF] = {0}, 
		ip[INET_ADDRSTRLEN] = {0};

	inet_ntop(AF_INET, &csock->sin_addr, ip, sizeof(ip));
	printf("__Start_NODE={%p}\n", (void *)n);
	printf("\tcsock{addr=%s, port=%d}\n", ip, port);
	printf("\tmsg={left %lu, written=%lu, buf=%s\n", \
		n->left, n->written, n->msg);
	rstate_show(&n->state);
	strftime(buf, MINI_BUF, "%Y-%m-%d %H:%M:%S", tm_info);
	printf("time={%s}\n", buf);
	printf("__End_NODE\n");
}

void
print_tree(RMsg_p n)
{
	RMsg_p left, right;

	if (n == NULL) {
		printf("nil");
		return;
	}
	left = RB_LEFT(n, entry);
	right = RB_RIGHT(n, entry);
	if (left == NULL && right == NULL)  {
		print_node(n);
	} else {
		print_node(n);
		print_tree(left);
		printf(",");
		print_tree(right);
		printf(")");
	}
}

void
print_tree_root(rhandle_port_p t)
{
	RMsg_p n = RB_ROOT(t);

	print_tree(n);
}

void
print_node_addr(RAddr_p n)
{
	char ip[INET_ADDRSTRLEN] = {0};

	inet_ntop(AF_INET, &n->addr, ip, sizeof(ip));
	printf("__Start_ADDR={%p}\n", (void *)n);
	printf("\tcsock{addr=%s}\n", ip);
	printf("\tncons{%d}\n", n->ncons);
	print_tree_root(&n->port_tree);
	printf("__End_ADDR\n");
}

void
print_tree_addr(RAddr_p n)
{
	RAddr_p left, right;

	if (n == NULL) {
		printf("nil");
		return;
	}
	left = RB_LEFT(n, entry);
	right = RB_RIGHT(n, entry);
	if (left == NULL && right == NULL) {
		print_node_addr(n);
	} else {
		print_node_addr(n);
		print_tree_addr(left);
		printf(",");
		print_tree_addr(right);
		printf(")");
	}
}

void
print_tree_root_addr(rhandle_tree_p t)
{
	RAddr_p n = RB_ROOT(t);

	printf("ADDR_TREE_S\n");
	print_tree_addr(n);
	printf("ADDR_TREE_E\n");
}

void *
csock_send(void *rmsg_v)
{
	RMsg_p rmsg = (RMsg_p)rmsg_v;
	int ret = -1;
	bool convert = false;

	bool switch_off = false;
	if (rmsg->active[ONA] == false && rmsg->active[CLR] == true) {
		fprintf(stderr, "ERROR: csock_send: inactive port\n");
		fflush(stderr);
		goto UNLOCK;
	} else if (rmsg->active[CLR] == false && rmsg->active[ONA] == false) {
		goto UNLOCK;
	}
	if ((FLAG_CHECK((rmsg->state.curr->hdr), (FIN|ACK)) == true) || \
		FLAG_CHECK((rmsg->state.curr->hdr), (FIN|ACK|CHK)) == true) {
#ifdef DEBUG_END
		printf("switchoff\n");
#endif //DEBUG_END
		switch_off = true;
	}
	convert = true;
	if (!(FLAG_CONFIRM(rmsg->state.curr->hdr))) {
#ifdef DEBUG_END
		fprintf(stderr, "ERR: %s\n", __func__);
		_rhdr_pretty_print(&rmsg->state.curr->hdr, stderr);
		convert_bytes(&rmsg->state.curr->hdr, true);
		rhead_fix_seq(&rmsg->state.states[MYS]);
		_rhdr_pretty_print(&rmsg->state.curr->hdr, stderr);
#endif //DEBUG_END
	}
	convert_bytes(&rmsg->state.curr->hdr, false);
	ret = sendto(sock, &rmsg->state.curr->hdr, \
		RHDR_SIZ, 0, (struct sockaddr *)&rmsg->csock, rmsg->clen);
	if (ret == -1) {
		switch (errno) {
			case EAGAIN:
				break;
			/*case EWOULDBLOCK:
				break;*/
			default:
				fprintf(stderr, \
					"send: %d %s\n", \
					ret, strerror(errno));
		}
	}
UNLOCK:
	if (switch_off == true) {
		rmsg->active[ONA] = false;
#ifdef DEBUG_END
		printf("false CLR,ONA={%d,%d}\n", rmsg->active[CLR], rmsg->active[ONA]);
#endif //DEBUG_END
	}
	if (convert == true) {
		convert_bytes(&rmsg->state.curr->hdr, true);
	}
	pthread_mutex_unlock(&rmsg->lock);
	return (NULL);
}

void *
rhandle_retransmit(void *nada __attribute__((unused)))
{
#ifdef DEBUG_FUNC
	printf("START: rhandle_retransmit\n");
#endif
	time_t freeze = time(NULL);
	RAddr_p addr = NULL, atmp = NULL;

	RB_FOREACH_SAFE(addr, rhandletree_addr, addr_tree_p, atmp) {
		RMsg_p port = NULL, ptmp = NULL;
		/*if (freeze - addr->update_stamp < MAX_TIMEOUT) {
			continue;
		}*/
		RB_FOREACH_SAFE(port, rhandletree_port, &addr->port_tree, ptmp) {
			if (port->active[ONA] == false && port->active[CLR] == false && \
				pthread_mutex_trylock(&port->lock) == 0) {
				rstate_destroy(&port->state);
				port->active[ONA] = false;
				port->active[CLR] = true;
				pthread_mutex_unlock(&port->lock);
			} else if (freeze - port->update_stamp[MYS] >= MAX_TIMEOUT && \
				port->active[ONA] == true && port->active[CLR] == false && \
				pthread_mutex_trylock(&port->lock) == 0) {
				if (freeze - port->update_stamp[HIS] >= DEAD_TIMEOUT) {
					port->active[ONA] = false;
				}
				printf("sending\n");
				port->update_stamp[MYS] = freeze;
				int ret = pthread_create(&port->stick, NULL, csock_send, port);
				if (ret != 0) {
					fprintf(stderr, "pthread_create: %d %s\n", ret, strerror(errno));
				} else {
					pthread_detach(port->stick);
				}
			}
		}
	}
#ifdef DEBUG_FUNC
	printf("FINISH: rhandle_retransmit\n");
#endif
	return NULL;
}

bool
rhandle_port(char *buf, size_t len, \
	struct sockaddr_in *csock, size_t clen, \
	rhandle_port_p p_tree)
{
#ifdef DEBUG_FUNC
	printf("START: rhandle_port\n");
#endif
	RMsg_p curr = (RMsg_p)malloc(sizeof(*curr)), \
		prev = NULL;
	time_t old_time;

	if (curr == NULL) {
		fprintf(stderr, "malloc: %d, %s", \
			errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(curr, 0, sizeof(*curr));
	RSTATE_INIT(curr->state);
	pthread_mutex_init(&curr->lock, NULL);
	curr->active[ONA] = true;
	curr->active[CLR] = false;

	curr->clen = clen;
	memcpy(&curr->csock, csock, clen);
	memcpy(&curr->port, &csock->sin_port, \
		sizeof(csock->sin_port));
	prev = RB_FIND(rhandletree_port, p_tree, curr);
	if (prev == NULL) { // curr inserted
		curr->nreqs[MSG_PR] += 1;
		memcpy(curr->msg, buf, len);
		curr->written = len;
		curr->left = RHDR_SIZ - len;
		curr->update_stamp[HIS] = \
			curr->update_stamp[MYS] = time(NULL);
		prev = RB_INSERT(rhandletree_port, p_tree, curr);
		prev = curr;
	} else { // already present free curr
		free(curr);
		prev->nreqs[MSG_PR] += 1;
		if (len >= RHDR_SIZ) {
			memcpy(prev->msg, buf, len);
			prev->left = 0;
			prev->written = RHDR_SIZ;
		} else {
			size_t space = (len > prev->left) ? prev->left : len;
			memcpy((prev->msg + prev->written), buf, space);
			prev->left -= space;
			prev->written += space;
		}
		old_time = prev->update_stamp[HIS];
		prev->update_stamp[HIS] = \
			prev->update_stamp[MYS] = time(NULL);
	}
	if (prev->active[CLR] == true) {
		RSTATE_INIT(prev->state);
		prev->active[ONA] = true;
		prev->active[CLR] = false;
		prev->clen = clen;
		memcpy(&prev->csock, csock, clen);
		memcpy(&prev->port, &csock->sin_port, \
			sizeof(csock->sin_port));
		fprintf(stderr, "ALERT: again from {}\n");
		//return false;
	}
	// if prev is full, handle the RHdr request
	if (pthread_mutex_lock(&prev->lock) != 0) {
		fprintf(stderr, \
			"pthread_mutex_lock: %d %s", \
			errno, strerror(errno));
	}
	if (prev->left == 0 && prev->written == RHDR_SIZ) {
		Entry e = rhead_append(&prev->state.states[HIS]);
		memcpy(&e->hdr, prev->msg, RHDR_SIZ);
		int rp_ret = rprocess_request(&prev->state);
		if (rp_ret == 2) {
			prev->update_stamp[HIS] = \
				prev->update_stamp[MYS] = old_time;
			goto UNLOCK_PORT;
			// return false;
		} else if (rp_ret == 3) {
			prev->active[CLR] = false;
			prev->active[ONA] = false;
			prev->nreqs[BAD_PR] += 1;
			goto UNLOCK_PORT;
			// return false;
		}
		if (rp_ret == false && \
				prev->state.curr == NULL) {
			prev->active[CLR] = false;
			prev->active[ONA] = false;
			prev->nreqs[BAD_PR] += 1;
		}
		if (rp_ret == false) {
#ifdef DEBUG_END
			printf("false CLR,ONA={%d,%d}\n", prev->active[CLR], prev->active[ONA]);
#endif //DEBUG_END
		}
		//bcopy(&prev->state.curr->hdr, buf, RHDR_SIZ);
		int ret = pthread_create(&prev->stick, NULL, csock_send, prev);
		if (ret != 0) {
			fprintf(stderr, "pthread_create: %d %s\n", \
					ret, strerror(errno));
		} else {
			pthread_detach(prev->stick);
		}
	}
#ifdef DEBUG_FUNC
	printf("FINISH: rhandle_port\n");
#endif
	return true;
UNLOCK_PORT:
	pthread_mutex_unlock(&prev->lock);
	return false;
}

bool
rhandle_serv(char *buf, size_t len, \
	struct sockaddr_in *csock, size_t clen)
{
	RAddr_p curr = \
		(RAddr_p)malloc(sizeof(*curr)), \
		prev = NULL;

	if (curr == NULL) {
		fprintf(stderr, \
			"malloc: %d, %s", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	memset(curr, 0, sizeof(*curr));
	curr->ncons = 1;

	// FORCE 0.0.0.0 to be 127.0.0.1
	if (csock->sin_addr.s_addr == 0) {
		csock->sin_addr.s_addr = LOCALHOST_ADDRV;
	}
	memcpy(&curr->addr, &csock->sin_addr, \
		sizeof(csock->sin_addr));
	prev = RB_FIND(rhandletree_addr, addr_tree_p, curr);
	if (prev == NULL) { // curr inserted
		curr->ncons += 1;
		curr->port_tree = \
			(rhandle_port_t)RB_INITIALIZER(curr->port_tree);
		curr->update_stamp = time(NULL);
		if (rhandle_port(buf, len, csock, clen, \
			&curr->port_tree) == false) {
			return false;
		}
		prev = RB_INSERT(rhandletree_addr, addr_tree_p, curr);
	} else { // already present free curr
		free(curr);
		prev->update_stamp = time(NULL);
		if (rhandle_port(buf, len, csock, clen, \
			&prev->port_tree) == false) {
			return false;
		}
	}
	return true;
}

void
rhandle_destroy(void)
{
	rhandle_retransmit(NULL);
	RAddr_p addr = NULL, atmp = NULL;
	RB_FOREACH_SAFE(addr, rhandletree_addr, addr_tree_p, atmp) {
		RMsg_p port = NULL, ptmp = NULL; 
		RB_FOREACH_SAFE(port, rhandletree_port, &addr->port_tree, ptmp) {
#if DEBUG_FREE
			printf("Freeing RMsg_p %p\n, tmp={%p}", \
				(void *)port, (void *)ptmp);
#endif
			RB_REMOVE(rhandletree_port, &addr->port_tree, port);
			pthread_mutex_lock(&port->lock);
			if (port->active[CLR] == false) {
				rstate_destroy(&port->state);
			}
			pthread_mutex_destroy(&port->lock);
			free(port);
		}
		RB_REMOVE(rhandletree_addr, addr_tree_p, addr);
		free(addr);
	}

	//pthread_exit(0);
}

#endif //__RHANDLESERV__H__
