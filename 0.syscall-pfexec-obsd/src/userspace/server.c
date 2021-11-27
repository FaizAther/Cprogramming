#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <err.h>
#include <errno.h>

int
main(void)
{
	struct sockaddr_un addr = {0};
	int fd = 0, ret = 0, cfd = 0;
	char buf[1024] = {0};
	struct sockpeercred creds = {0};
	socklen_t credslen = sizeof(creds);
	int error = 0;

	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd < 0) {
		err(1, "socket");
	}
	
	addr.sun_len = sizeof addr;
	addr.sun_family = AF_UNIX;
	ret = strlcpy(addr.sun_path, "/tmp/foobar", sizeof addr.sun_path);

	ret = bind(fd, (struct sockaddr *)&addr, sizeof addr);
	if (ret < 0) {
		err(1, "bind");
	}

	ret = listen(fd, 8);
	if (ret < 0) {
		err(1, "listen");
	}
	
	cfd = accept(fd, NULL, NULL);
	if (cfd < 0) {
		err(1, "accept");
	}
	
	ret = read(cfd, buf, sizeof buf - 1);
	printf("sizeof buf: %lu, read %s\n", sizeof buf - 1, buf);

	error = getsockopt(cfd, SOL_SOCKET, SO_PEERCRED, &creds, &credslen);
	printf("client pid %d\n", creds.pid);

	close(fd);
	return (0);
}
