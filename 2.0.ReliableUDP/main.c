#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "RHdr.h"
#include "RQueue.h"
#include "RState.h"

void
run_timer(void)
{
	time_t update_stamp[2], now;
	memset(update_stamp, 0, sizeof(update_stamp));

	update_stamp[0] = time(NULL);

	sleep(4);

	char buf[26];
	now = time(NULL) - update_stamp[0];
	struct tm *tm_info = localtime(&now);
	strftime(buf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
	printf("time={%s}, diff={%lu}\n", buf, now);
}

void
show_bytes(Data bytes[MSG_SIZ]) {
	for(uint16_t i = 0; i < MSG_SIZ || bytes[i] == '\0'; i++) {
		printf("%u: %u\n", i, (uint8_t)bytes[i]);
	}
	printf("\n");
}

void
check_encryption(void)
{
	RHdr_t hdr;
	RHDR_INIT(hdr);

	Data bytes[MSG_SIZ] = {(uint8_t)247, (uint8_t)181, (uint8_t)86, (uint8_t)181, 0};
	// bzero(&bytes, MSG_SIZ);
	// for (uint8_t i = 0; i < 249; i++) {
	// 	bytes[i] = i;
	// }
	// bytes[0] = (uint8_t)255;
	memcpy(hdr.msg, bytes, MSG_SIZ);

	//show_bytes(hdr.msg);
	rhdr_computer_internal(&hdr, decrypt_);
	// show_bytes(hdr.msg);
	printf("%s\n", hdr.msg);
	rhdr_computer_internal(&hdr, encrypt_);
	show_bytes(hdr.msg);
}

int
main(void)
{
	RHdr_t hdr = rhdr_whitetest_run();

	//run_timer();
	// rstate_whitetest_run();
	rhdr_pretty_print(&hdr);

	//rhead_whitetest_run();


	//assert(0x38d6 == rhdr_computer_chksum(&hdr));

	return (0);
}
