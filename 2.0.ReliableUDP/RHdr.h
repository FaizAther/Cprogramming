#ifndef __RHDR__H__
#define __RHDR__H__

/*
 * ather
 * The University of Queensland, 2022
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>

#include "RKey.h"

#define R_VERSION 	0x0002
#define NVALS		4U
#define RHDR_SIZ	1472U
#define RHDR_VA_SZ	8U

#define MSG_SIZ		1464
#define MSG_BUF		(MSG_SIZ + 1)
#define FLAG_OFFSET	0xFE00

#define EMPTY_CHECKSUM_PRECOMPUTE \
			65535

#define STR_(X) #X
#define STR(X) STR_(X)

#define FLAG_VERSION_INIT(hdr) \
	hdr.content[FLGV] = 0U; \
	hdr.content[FLGV] = R_VERSION;

#define RHDR_INIT(hdr) \
	memset(&hdr, 0, sizeof(hdr)); \
	FLAG_VERSION_INIT(hdr);

#define FLAG_SET(hdr, flag) \
	hdr.content[FLGV] |= (flag & FLAG_OFFSET);

#define FLAG_UNSET(hdr, flag) \
	hdr.content[FLGV] &= (~(flag & FLAG_OFFSET));

#define FLAG_GET(hdr) \
	(hdr).content[FLGV]
	//((hdr).content[FLGV] & (FLAG_OFFSET|R_VERSION))

#define FLAG_CHECK(hdr, flag) \
	((FLAG_GET(hdr) == (flag|R_VERSION)) ? true : false)

#define FLAG_VALIDATE(hdr, flag) \
	(((FLAG_GET(hdr) & (flag|R_VERSION)) == (flag|R_VERSION)) ? true : false)

#define FLAG_CONFIRM(hdr) \
	((FLAG_GET((hdr)) & 0x03FF) == R_VERSION)

#define VERSION_GET(hdr) \
	(hdr.content[FLGV] & 7)

#define RHDR_MSG_SET(hdr, msg_str, msg_len) \
	memcpy(hdr.msg, msg_str, \
                (msg_len < MSG_SIZ) ? msg_len : MSG_SIZ)

#define RHDR_CONTENT_SET(hdr, cno, val) \
	((hdr).content[cno]) = val;
/*
enum flags {
	ACK = (1 << 0),
	NAK = (1 << 1),
	GET = (1 << 2),
	DAT = (1 << 3),
	FIN = (1 << 4),
	CHK = (1 << 5),
	ENC = (1 << 6)
};
*/
enum flags {
	ACK = 0x8000,
	NAK = 0x4000,
	GET = 0x2000,
	DAT = 0x1000,
	FIN = 0x0800,
	CHK = 0x0400,
	ENC = 0x0200,
};

enum vals {
	SEQN = 0U,
	ACKN = 1U,
	CHKS = 2U,
	FLGV = 3U
};

typedef char Data;
typedef uint16_t RHdrCon_t;

typedef struct RHdr_t {
	RHdrCon_t content[NVALS];
	Data msg[MSG_SIZ];
} RHdr_t;
typedef RHdr_t *RHdr_p;

void
_rhdr_pretty_print(RHdr_p hdr, FILE *stream)
{
	fprintf(stream, "RHdr___start\n");

	fprintf(stream, "\tseq_no={%u}\n", hdr->content[SEQN]);
	fprintf(stream, "\tack_no={%u}\n", hdr->content[ACKN]);
	fprintf(stream, "\tchk_sum={%u}\n", hdr->content[CHKS]);
	fprintf(stream, "\tflag_version={%u, %x} ", hdr->content[FLGV], hdr->content[FLGV]);
	if (FLAG_CHECK((*hdr), ACK)) {
		fprintf(stream, "ACK,");
	}
	if (FLAG_VALIDATE((*hdr), NAK)) {
		fprintf(stream, "NAK,");
	}
	if (FLAG_VALIDATE((*hdr), GET)) {
		fprintf(stream, "GET,");
	}
	if (FLAG_VALIDATE((*hdr), DAT)) {
		fprintf(stream, "DAT,");
	}
	if (FLAG_VALIDATE((*hdr), FIN)) {
		fprintf(stream, "FIN,");
	}
	if (FLAG_VALIDATE((*hdr), CHK)) {
		fprintf(stream, "CHK,");
	}
	if (FLAG_VALIDATE((*hdr), ENC)) {
		fprintf(stream, "ENC,");
	}
	fprintf(stream, "\n\tflag={%u}\n", FLAG_GET((*hdr)));

	fprintf(stream, "\tmsg={%." STR(MSG_SIZ) "s}\n", hdr->msg);

	fprintf(stream, "RHdr___end\n");
}

void
rhdr_pretty_print(RHdr_p hdr)
{
	_rhdr_pretty_print(hdr, stdout);
}

void
rhdr_whitetest(RHdr_p hdr_p)
{
	for (uint8_t i = SEQN; i < NVALS; i++) {
		printf("content[%u]={%d}\n", i, hdr_p->content[i]);
	}

	printf("flag_version={%u}, flag={%u}\n", \
		hdr_p->content[FLGV], FLAG_GET((*hdr_p)));
	FLAG_SET((*hdr_p), NAK);
	printf("flag_version={%u}, flag={%u}\n", \
		hdr_p->content[FLGV], FLAG_GET((*hdr_p)));
	printf("check(hdr, NAK)={%u}\n", FLAG_CHECK((*hdr_p), NAK));
	printf("check(hdr, ACK)={%u}\n", FLAG_CHECK((*hdr_p), ACK));
	printf("sizeof(RHdr)={%lu}, NAK={%d}, " \
		"flag={%u}, sizeof(flag)={%lu}, sizeof(ACK)={%lu}\n", \
		sizeof(RHdr_t), NAK, hdr_p->content[FLGV], \
		sizeof(hdr_p->content[FLGV]), sizeof(ACK));
}

static RHdrCon_t
computer_chksum( \
	Data msg[MSG_SIZ] __attribute__((unused)), \
	RHdrCon_t pos __attribute__((unused)), \
	Data c1, Data c2)
{
	RHdrCon_t calc = (RHdrCon_t)c1 + ((RHdrCon_t)c2 << 8);
#if DEBUG_CHK
	printf("%x%x\n", c2, c1);
	printf("%c%c\n", c2, c1);
	printf("w 0x%x %c %c\n", calc, c1, c2);
#endif // CHECKSUM
	return calc;
}

RHdrCon_t
carry_around_add(RHdrCon_t sum, RHdrCon_t val)
{
	uint32_t over = sum + val;
	return (over & 0xffff) + (over >> 16);
}

RHdrCon_t
rhdr_computer_internal(RHdr_p hdr_p, \
	RHdrCon_t compute_func(Data[MSG_SIZ], RHdrCon_t, Data, Data))
{
	RHdrCon_t accum = 0;
	uint16_t i = 0;

	while (i < MSG_SIZ - 1 && \
		hdr_p->msg[i] != '\0') {
		accum = carry_around_add(accum, \
			compute_func( hdr_p->msg, \
				i, \
				hdr_p->msg[i], \
				hdr_p->msg[i + 1]));
		i += 2;
	}
	return ~accum;
}

RHdrCon_t
rhdr_computer_chksum(RHdr_p hdr_p)
{
	return rhdr_computer_internal( \
		hdr_p, computer_chksum);
}

bool
valid_chksum(RHdr_p hdr_p)
{
	if (FLAG_VALIDATE((*hdr_p), CHK) == false) {
		return false;
	}
	if (rhdr_computer_chksum(hdr_p) == hdr_p->content[CHKS]) {
		return true;
	}

	return false;
}

bool
client_chksum(RHdr_p hdr_p)
{
	if (FLAG_VALIDATE((*hdr_p), CHK) == false) {
		return false;
	}
	if (!((FLAG_CHECK((*hdr_p), (GET)) == true) || (FLAG_CHECK((*hdr_p), (GET|CHK)) == true))) {
		if (hdr_p->content[CHKS] != EMPTY_CHECKSUM_PRECOMPUTE) {
			return false;
		}
	}
	return valid_chksum(hdr_p);
}

static uint8_t
compute_crypt(uint8_t ch, uint64_t po, uint64_t num)
{
	uint64_t chi = ch;
	chi = powr((uint64_t)ch, po, num);
	return (uint8_t)chi;
}

static RHdrCon_t
encrypt_( \
	Data msg[MSG_SIZ] __attribute__((unused)), \
	RHdrCon_t pos __attribute__((unused)), \
	Data c1, Data c2)
{
	msg[pos] = compute_crypt(c1, E, N);
	msg[pos + 1] = compute_crypt(c2, E, N);
	return 0;
}

RHdrCon_t
encrypt(RHdr_p hdr_p) {
	return rhdr_computer_internal( \
		hdr_p, encrypt_);
}

static RHdrCon_t
decrypt_( \
	Data msg[MSG_SIZ] __attribute__((unused)), \
	RHdrCon_t pos __attribute__((unused)), \
	Data c1, Data c2)
{
	msg[pos] = compute_crypt(c1, D, N);
	msg[pos + 1] = compute_crypt(c2, D, N);
	return 0;
}

RHdrCon_t
decrypt(RHdr_p hdr_p) {
	return rhdr_computer_internal( \
		hdr_p, decrypt_);
}

void
convert_bytes(RHdr_p hdr_p, bool network)
{
	for (uint8_t i = 0; i < 4; i++) {
		if (network == true) {
			hdr_p->content[i] = ntohs(hdr_p->content[i]);
		} else {
			hdr_p->content[i] = htons(hdr_p->content[i]);
		}
	}
}

RHdr_t
rhdr_whitetest_run(void)
{
	RHdr_t hdr;
	RHDR_INIT(hdr);
	//rhdr_whitetest(&hdr);
	FLAG_SET(hdr, CHK);
	//FLAG_SET(hdr, NAK);
	printf("falg_get={%x}, GET|NAK={%x}, ((GET|NAK) & (GET|NAK))={%x}, ((GET|NAK) & (GET|ACK))={%x}, flag_check(GET|NAK)={%x}, flag_check(GET|ACK)={%x} flag_validate(GET)={%x}, \n", \
		FLAG_GET(hdr), (GET|NAK), ((GET|NAK) & (GET|NAK)), ((GET|NAK) & (GET|ACK)), FLAG_CHECK(hdr, (GET|NAK)), FLAG_CHECK(hdr, (GET|ACK)), FLAG_VALIDATE(hdr, GET));
	// RHDR_MSG_SET(hdr, "files/file.txt", sizeof("files/file.txt"));
	RHDR_MSG_SET(hdr, "abcde\0", sizeof("abcde\0"));
	RHdrCon_t sum = rhdr_computer_chksum(&hdr);
	printf("chksum(\"abcde\")={%x, %u}\n", sum, sum);
	// rhdr_computer_internal(&hdr, encrypt_);
	// for (int i = 0; hdr.msg[i] != '\0'; i++) {
	// 	printf("%u,", (uint8_t)hdr.msg[i]);
	// }
	printf("\n");
	//printf("hdr.msg={%s}\n", hdr.msg);
	//rhdr_computer_internal(&hdr, decrypt_);
	printf("hdr.msg={%s}\n", hdr.msg);
	printf("%u\n", (GET) & 127U);
	return hdr;
}

#endif //__RHDR__H__
