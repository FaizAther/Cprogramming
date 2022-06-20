#include <stdio.h>

#include <stdbool.h>

#include <stdint.h>

#include <arpa/inet.h>

#include "RSPkt.h"

uint8_t
longest_prefix_mathing(uint32_t a, uint32_t b)
{
	a = ntohl(a);
	b = ntohl(b);

	uint8_t matched = 0;
	for (uint8_t i = 31; i >= 0; i--) {
		bool x = false;
		uint32_t ax = (a & (uint32_t)(1 << i));
		uint32_t bx = (b & (uint32_t)(1 << i));
		if (ax == bx) { x = true; }
		if (!x) { return matched; }
		matched += 1;
	}
	return matched;
}

int
main(void)
{
  uint32_t a = 0/*0b10000001 << 24*/;
  uint32_t b = 0/*0b10000111 << 24*/;
  uint32_t c = 0/*0b10001000 << 24*/;

  rsPkt_ip_op_put(&a, "129.0.0.1");
  rsPkt_ip_op_put(&b, "135.0.0.1");
  rsPkt_ip_op_put(&c, "136.0.0.1");

  uint8_t longest_a_b = longest_prefix_mathing(a, b);
  uint8_t longest_a_c = longest_prefix_mathing(a, c);

  printf("ab=%u, ac=%u\n", longest_a_b, longest_a_c);

  return 0;
}