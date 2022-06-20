#ifndef __RIPOP__H__
#define __RIPOP__H__

/*
	ather
	coms3200, The University of Queensland 2022
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IP_BOX_SZ 4
#define IP_MAX_SPACE 32

#define IP_SUBNET_MASK 255

#define IP_MAX_NAME 16

#define RIP_DEFAULT_IP_CHECK "192.168.0.1"
#define RIP_DEFAULT_SUBNET_CHECK 24

enum ip_box {
	fst = 0,
	snd,
	thd,
	frt,
} ip_box;

enum ip_bit {
	bi_fi = 8,
	bi_se = 16,
	bi_th = 24,
	bi_fo = 32,
} ip_bit;

typedef struct RIPsingle_t {
	uint8_t ip_boxp[IP_BOX_SZ];
} RIPsingle_t;
typedef RIPsingle_t *RIPsingle;

typedef struct RIPop_t {
	uint8_t ip_box[IP_BOX_SZ];
	uint8_t ip_cur[IP_BOX_SZ];
	uint8_t ip_max[IP_BOX_SZ];
	uint8_t space;
} RIPop_t;
typedef RIPop_t *RIPop;

#define RIPOP_INIT(op) \
	memset(op, 0, sizeof(struct RIPop_t));

void
rIpop_op_to_str(RIPsingle ip, char buf[IP_MAX_NAME])
{
	char *where = buf;
	for (uint8_t i = 0; i < IP_BOX_SZ; i++) {
		int wrote = snprintf(where, IP_BOX_SZ + 1, "%u.", ip->ip_boxp[i]);
		where += wrote;
		if (i == 3) { *(where - 1) = '\0'; };
	}
}

bool
rIpop_assign_ip(RIPop op, RIPsingle ip, bool increment)
{
	memcpy(ip, op->ip_cur, sizeof(struct RIPsingle_t));
	if (op->ip_cur[frt] < IP_SUBNET_MASK) {
		ip->ip_boxp[frt] += 1;
	} else if (op->ip_cur[thd] < IP_SUBNET_MASK) {
		ip->ip_boxp[frt] = 0;
		ip->ip_boxp[thd] += 1;
	} else if (op->ip_cur[snd] < IP_SUBNET_MASK) {
		ip->ip_boxp[thd] = 0;
		ip->ip_boxp[snd] += 1;
	} else if (op->ip_cur[fst] < IP_SUBNET_MASK) {
		ip->ip_boxp[snd] = 0;
		ip->ip_boxp[fst] += 1;
	}

	if (increment) {
		if (ip->ip_boxp[frt] <= op->ip_max[frt] && \
			ip->ip_boxp[snd] <= op->ip_max[snd] && \
			ip->ip_boxp[thd] <= op->ip_max[thd] && \
			ip->ip_boxp[fst] <= op->ip_max[fst]) {
			memcpy(op->ip_cur, ip, sizeof(struct RIPsingle_t));
			return true;
		} else {
			memset(ip, 0, sizeof(struct RIPsingle_t));
			return false;
		}
	}
	return true;
}

void
rIpop_justify(RIPop op, uint8_t space)
{
	space = IP_MAX_SPACE - space;
	for (uint8_t i = 0; ((i < IP_MAX_SPACE) && (i < space)); i += 1) {
		if (i < bi_fi) {
			op->ip_max[frt] |= 1 << (i - 0);
		} else if (i >= bi_fi && i < bi_se) {
			op->ip_max[thd] |= 1 << (i - bi_fi);
		} else if (i >= bi_se && i < bi_th) {
			op->ip_max[snd] |= 1 << (i - bi_se);
		} else if (i >= bi_th && i < bi_fo) {
			op->ip_max[fst] |= 1 << (i - bi_th);
		}
	}
	for (uint8_t i = fst; i < IP_BOX_SZ; i += 1) {
		if (op->ip_max[i] == 0) { op->ip_max[i] = op->ip_cur[i]; }
	}
	op->space = space;
}

bool
rIpop_str_validate_good(char *name, uint8_t *name_sz)
{
	if (name == NULL) { return false; }
	int dots = 0, digits = 0;
	bool bad = false;
	if (name == NULL) { return false; }
	if (name_sz != NULL) { *name_sz = 0; }
	for (uint8_t i = 0; i < IP_MAX_NAME && !bad; i++) {
		if (name_sz != NULL) { *name_sz = *name_sz + 1; }

		if (name[i] == '\0') { break; }
		if (name[i] == '.') { dots++; }
		if (name[i] != '.' && (name[i] < '0' || name[i] > '9')) { bad = true; }
		if (name[i] >= '0' || name[i] <= '9') { digits++; }
		if (i > 0 && name[i] == '.' && name[i - 1] == '.') { bad = true; }
	}
	if (dots == 3 && digits >= 4 && digits <= 12 && !bad) { return true; }
	return false;
}

bool
rIpop_str_to_op(RIPop op, char name[IP_MAX_NAME])
{
	fprintf(stderr, "%s\n", name);
	uint8_t s = 0, x = fst;
	char re = 0;

	for (uint8_t i = 0; i < IP_MAX_NAME; i++) {
		if (name[i] == '.' || name[i] == '\0') {
			if (name[i] == '\0')
				re = '\0';
			else
				re = '.';
			name[i] = '\0';
			op->ip_cur[x] = op->ip_box[x] = (uint8_t)atoi(name + s);
			fprintf(stderr, "%s = %d\n", name + s, atoi(name + s));
			name[i] = re;
			s = i + 1;
			x += 1;
		}
		if (name[i] == '\0' || name[i] == 0) {
			break;
		}
	}
	return true;
}

void
rIpop_show(RIPop op, FILE *stream)
{
	fprintf(stream, "\tip_box{\n");
	for (uint8_t i = 0; i < IP_BOX_SZ; i += 1) {
		fprintf(stream, \
			"\t\t%u@{ %u },\t%u@hex{ 0x%02x }\n", \
			i, op->ip_box[i], i, op->ip_box[i]);
	}
	fprintf(stream, "\tip_cur{\n");
	for (uint8_t i = 0; i < IP_BOX_SZ; i += 1) {
		fprintf(stream, \
			"\t\t%u@{ %u },\t%u@hex{ 0x%02x }\n", \
			i, op->ip_cur[i], i, op->ip_cur[i]);
	}
	fprintf(stream, "\t} ip_max{\n");
	for (uint8_t i = 0; i < IP_BOX_SZ; i += 1) {
		fprintf(stream, \
			"\t\t%u@{ %u },\t%u@hex{ 0x%02x }\n", \
			i, op->ip_max[i], i, op->ip_max[i]);
	}
	fprintf(stream, \
		"\t} ip_space{ %u },\tip_space_hex{ 0x%02x }\n", \
		op->space, op->space);
}

static void
rIpop_whitetest(int subnet)
{
	RIPop_t op = {0};
	RIPop op_p = &op;
	RIPOP_INIT(op_p);
	char name[IP_MAX_NAME];
	memcpy(name, RIP_DEFAULT_IP_CHECK, sizeof(RIP_DEFAULT_IP_CHECK));
	rIpop_str_to_op(op_p, name);
	rIpop_justify(op_p, subnet < 0 ? RIP_DEFAULT_SUBNET_CHECK : subnet);
	rIpop_show(op_p, stderr);

	FILE *stream = fopen("ipassing.txt", "w");

	RIPsingle_t ip;
	for (int i = 0; i < 550; i++) {
		rIpop_assign_ip(op_p, &ip, false);
		// for (uint8_t i = 0; i < IP_BOX_SZ; i += 1) {
		// 	fprintf(stream, "\t%u", ip.ip_boxp[i]);
		// }
		char buf[IP_MAX_NAME];
		rIpop_op_to_str(&ip, buf);
		fprintf(stream, "\t%s", buf);
		fprintf(stream, "\n");
	}
	fclose(stream);

	return;
}

#endif //__RIPOP__H__