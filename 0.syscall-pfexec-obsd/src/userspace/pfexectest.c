#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <err.h>

#include <sys/pfexecvar.h>

int
main(void)
{
	struct pfexec_req req = {0};
	struct pfexec_resp resp = {0};
	struct sockaddr_un addr = {0};
	int fd = 0, ret = 0, slen = 0;

	req.pfr_pid = getpid();
	req.pfr_uid = getuid();
	req.pfr_gid = getgid();
	req.pfr_ngroups = getgroups(0, NULL);
//	req.pfr_groups

	req.pfr_req_flags = PFRESP_UID;
	ret = strlcpy(req.pfr_req_user, "root", sizeof "root");
	ret = strlcpy(req.pfr_path, "/bin/ls", sizeof "/bin/ls");

	fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd < 0) {
		err(1, "socket");
	}

	slen = sizeof req;
	ret = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &slen, sizeof slen);
	if (ret < 0) {
		close(fd);
		err(1, "setsockopt");
	}
	
	slen = sizeof resp + 32;
	ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &slen, sizeof slen);
	if (ret < 0) {
		close(fd);
		err(1, "setsockopt");
	}

	addr.sun_len = sizeof addr;
	addr.sun_family = AF_UNIX;
	ret = strlcpy(addr.sun_path, "/var/run/pfexecd.sock", sizeof addr.sun_path);

	ret = connect(fd, (struct sockaddr *)&addr, sizeof addr);
	if (ret < 0) {
		err(1, "connect");
	}

	ret = write(fd, &req, sizeof req);
	printf("write ret: %d\n", ret);

	ret = read(fd, (void *)&resp, sizeof resp);
	printf("read resp: %d, resp size: %lu, resp.pfr_errno: %d\n", ret, sizeof resp, resp.pfr_errno);

	close(fd);
	return (0);
}
