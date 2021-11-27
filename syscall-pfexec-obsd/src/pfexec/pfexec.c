/*
 * Copyright 2021, ather
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <unistd.h>
#include <stdlib.h>

#include <pwd.h>

#include <err.h>
#include <errno.h>

#include <limits.h>
#include <sys/stat.h>
#include <sys/pfexec.h>

extern char *optarg;
extern int optind;

static void __dead
usage(void)
{
	fprintf(stderr, "Usage:\tpfexec [options] <executable> [args...]\n"
	    "\tpfexec [options] -s\n"
	    "Options:\n"
	    " -u user\tAssume the privileges of the given user"
	    " instead of \"root\"\n"
	    " -n\t\tNon-interactive/batch mode:"
	    " no prompts are used\n"
	    "\t\tif prompts are required, will fail\n"
	    " -s\t\tExecutes a shell, rather than"
	    " a specifed command\n");

	exit(1);
}

static int
parseuid(const char *s, uid_t *uid)
{
	struct passwd *pw;
	const char *errstr;

	if ((pw = getpwnam(s)) != NULL) {
		*uid = pw->pw_uid;
		if (*uid == UID_MAX)
			return -1;
		return 0;
	}
	*uid = strtonum(s, 0, UID_MAX - 1, &errstr);
	if (errstr)
		return -1;
	return 0;
}

int
findprog(char *prog, char *path, char *ret_path)
{
	char *p, filename[PATH_MAX];
	int len, rval = 0;
	struct stat sbuf;
	char *pathcpy;

	/* To make access(2) do what we want */
	if (setgid(getgid()))
		err(1, "Can't set gid to %u", getegid());
	if (setuid(getuid()))
		err(1, "Can't set uid to %u", geteuid());


	/* Special case if prog contains '/' */
	if (strchr(prog, '/')) {
		if ((stat(prog, &sbuf) == 0) && S_ISREG(sbuf.st_mode) &&
		    access(prog, X_OK) == 0) {
			strlcpy(ret_path, prog, PATH_MAX);
			return (1);
		} else {
			return (0);
		}
	}

	if ((path = strdup(path)) == NULL)
		err(1, "strdup");
	pathcpy = path;

	while ((p = strsep(&pathcpy, ":")) != NULL) {
		if (*p == '\0')
			p = ".";

		len = strlen(p);
		while (len > 0 && p[len-1] == '/')
			p[--len] = '\0';	/* strip trailing '/' */

		len = snprintf(filename, sizeof(filename), "%s/%s", p, prog);
		if (len < 0 || len >= sizeof(filename)) {
			warnc(ENAMETOOLONG, "%s/%s", p, prog);
			free(path);
			return (0);
		}
		if ((stat(filename, &sbuf) == 0) && S_ISREG(sbuf.st_mode) &&
		    access(filename, X_OK) == 0) {
			rval = 1;
			strlcpy(ret_path, filename, PATH_MAX);
			free(path);
			return (rval);
		}
	}
	(void)free(path);

	return (rval);
}

int
main(int argc, char **argv, char **envp)
{
	struct pfexecve_opts opts = {0};
	uid_t e_uid = 0;
	gid_t e_gid = 0;
	int ret = 0, ch = 0, sflag = 0;
	char *safepath = "/bin:/sbin:/usr/bin:/usr/sbin:"
	    "/usr/local/bin:/usr/local/sbin";
	char *path, *sh;
	char *shargv[] = {NULL, NULL};
	char pass_path[PATH_MAX] = {0};

	strlcpy(opts.pfo_user, "root", sizeof("root"));

	while ((ch = getopt(argc, argv, "u:ns")) != -1) {
		switch (ch) {
		case 'u':
			opts.pfo_flags |= PFEXECVE_USER;
			if (parseuid(optarg, &e_uid) != 0)
				errx(1, "unknown user");
			bzero(opts.pfo_user, sizeof(opts.pfo_user));
			strlcpy(opts.pfo_user, optarg, \
			    sizeof(opts.pfo_user) - 1);
			break;
		case 'n':
			opts.pfo_flags |= PFEXECVE_NOPROMPT;
			break;
		case 's':
			sflag = 1;
			break;
		default:
			usage();
			break;
		}
	}

	argv += optind;
	argc -= optind;

	if (argc == 0 && !sflag) {
		errx(1, "No arguments to be parsed");
	}

	if (sflag) {
		sh = getenv("SHELL");
		if (sh == NULL || *sh == '\0') {
			shargv[0] = "/bin/ksh";
		} else
			shargv[0] = sh;
		argv = shargv;
		argc = 1;
	}

	if ((path = getenv("PATH")) == NULL || *path == '\0') {
		path = safepath;
	}

	bzero(pass_path, sizeof(pass_path));
	if (findprog(argv[0], path, pass_path) == 0) {
		errx(1, "%s: Command not found.", argv[0]);
	}

	ret = pfexecvpe(&opts, pass_path, argv, envp);
	if (ret) {
		fprintf(stderr, "pfexec: pfexecvpe: %s\n", strerror(errno));
	}

	return (ret);
}
