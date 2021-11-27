#include <stdio.h>
#include <sys/pfexec.h>
#include <string.h>
#include <err.h>
#include <errno.h>

int
main(int argc, char **argv, char **envp)
{
	const struct pfexecve_opts opts = {
	    .pfo_flags = 0,
	    .pfo_user = "root"
	};

	int pf = pfexecve(&opts, argv[1], argv, envp);
//	int pf = pfexecve(NULL, NULL, NULL, NULL);
	if (pf < 0) {
		fprintf(stderr, "pfexecve: %s\n", strerror(errno));
	}		
	printf("pf :%d\n", pf);
	return (0);
}
