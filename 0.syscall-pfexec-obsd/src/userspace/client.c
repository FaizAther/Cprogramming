#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <err.h>

int
main(void)
{
	struct sockaddr_un addr = {0};
	int fd = 0, ret = 0;

	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd < 0) {
		err(1, "socket");
	}
	
	addr.sun_len = sizeof addr;
	addr.sun_family = AF_UNIX;
	ret = strlcpy(addr.sun_path, "/tmp/foobar", sizeof addr.sun_path);

	ret = connect(fd, (struct sockaddr *)&addr, sizeof addr);
	if (ret < 0) {
		err(1, "bind");
	}
	write(fd, "hi", 2);
	printf("mypid: %d\n", getpid());

	return (0);
}
