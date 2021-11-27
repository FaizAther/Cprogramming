/*
 * Copyright 2021, the University of Queensland
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

#if !defined(_SYS_PFEXECVAR_H_)
#define	_SYS_PFEXECVAR_H_

#include <sys/types.h>
#include <sys/syslimits.h>

/* Path to the socket which pfexecd should listen on for kernel reqs */
#define	PFEXECD_SOCK	"/var/run/pfexecd.sock"

/* A reference to an argument or env var in one of the *area members */
struct pfexec_arg {
	uint32_t		pfa_offset;
	uint32_t		pfa_len;	/* not including NUL */
};

/* Request, sent from kernel to pfexecd. */
struct pfexec_req {
	/* information about calling process */
	uint32_t		pfr_pid;
	uint32_t		pfr_uid;
	uint32_t		pfr_gid;
	uint32_t		pfr_ngroups;
	uint32_t		pfr_groups[NGROUPS_MAX];

	/* extra info provided to the pfexecve() syscall by userland */
	uint32_t		pfr_req_flags;
	char			pfr_req_user[LOGIN_NAME_MAX];

	/* exec arguments from userland */
	char			pfr_path[PATH_MAX];
	uint32_t		pfr_argc;
	struct pfexec_arg	pfr_argp[1024];
	uint32_t		pfr_envc;
	struct pfexec_arg	pfr_envp[1024];
	char			pfr_argarea[ARG_MAX];
	char			pfr_envarea[ARG_MAX];
};

enum pfexec_resp_flags {
	PFRESP_UID	= (1<<0),
	PFRESP_GID	= (1<<1),
	PFRESP_GROUPS	= (1<<2),
	PFRESP_CHROOT	= (1<<3),
	PFRESP_ENV	= (1<<4)
};

/* Response, sent from pfexecd back to kernel. */
struct pfexec_resp {
	uint32_t		pfr_flags;	/* enum pfexec_resp_flags */

	/* The errno to return to userland (if any). Always used. */
	uint32_t		pfr_errno;

	/*
	 * Things to change about the target process. Each is valid if the
	 * matching bit in pfr_flags is set.
	 */
	uint32_t		pfr_uid;

	uint32_t		pfr_gid;

	uint32_t		pfr_ngroups;
	uint32_t		pfr_groups[NGROUPS_MAX];

	char			pfr_chroot[PATH_MAX];

	uint32_t		pfr_envc;
	struct pfexec_arg	pfr_envp[1024];
	char			pfr_envarea[ARG_MAX];
};

#endif /* !defined(_SYS_PFEXECVAR_H_) */
