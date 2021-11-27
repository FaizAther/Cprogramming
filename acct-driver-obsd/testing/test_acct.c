#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>
#include <err.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <sys/acct.h>
#include <dev/acct.h>

void
test_kern_acct(void)
{
	int ret = 0,
	    fd = 0;
	char *fname = "./somefile.txt";
	struct acct ac = {0};

	fd = open(fname, O_RDONLY);

	ret = read(fd, &ac, sizeof(ac));
	printf("ac_mem: %d\n", ac.ac_mem);

	if( access( fname, F_OK ) != -1 ) {
	    // file exists
	    ret = acct(fname);
	    printf("acct: %d\n", ret);
	} else {
	    // file doesn't exist
	}
}


void
test_acct(void)
{
	struct acct_ctl ctl;
	bzero(&ctl, sizeof(ctl));

	int fd = open("/dev/acct0", O_RDWR | O_EXLOCK);
	int rc = ioctl(fd, ACCT_IOC_STATUS, &ctl);

	printf("%d, %llu\n", ctl.acct_ena, ctl.acct_fcount);
	close(fd);
}

int
main(void)
{
	test_acct();
	return (0);
}
