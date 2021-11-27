/*	$OpenBSD$ */

/*
 * Copyright (c) 2021 The University of Queensland
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

#include <sys/types.h>
#include <sys/syslimits.h>
#include <sys/ioccom.h>

#ifndef __DEV_ACCT_H__
#define __DEV_ACCT_H__

#define ACCT_MSG_FORK	0
#define ACCT_MSG_EXEC	1
#define ACCT_MSG_EXIT	2
#define	ACCT_MSG_OPEN	3
#define	ACCT_MSG_RENAME	4
#define	ACCT_MSG_UNLINK	5
#define	ACCT_MSG_CLOSE	6

struct acct_common {
	unsigned short		ac_type;
	unsigned short		ac_len;
	unsigned int		ac_seq;

	char			ac_comm[16];	/* command name */
	struct timespec		ac_etime;	/* elapsed time */
	struct timespec		ac_btime;	/* starting time */
	pid_t			ac_pid;		/* process id */
	uid_t			ac_uid;		/* user id */
	gid_t			ac_gid;		/* group id */
	dev_t			ac_tty;		/* controlling tty */
	unsigned int		ac_flag;	/* accounting flags */
};

/*
 * fork info is mostly from the parent, but acct_fork gets passed the child.
 */
struct acct_fork {
	struct acct_common	ac_common;
	pid_t			ac_cpid;	/* child pid */
};

/*
 * exec exists mostly to show the new command name.
 */
struct acct_exec {
	struct acct_common	ac_common;
};

/*
 * basically a clone of the ACCOUNTING syscall
 */
struct acct_exit {
	struct acct_common	ac_common;
	struct timespec		ac_utime;	/* user time */
	struct timespec		ac_stime;	/* system time */
	uint64_t		ac_mem;		/* average memory usage */
	uint64_t		ac_io;		/* count of IO blocks */
};

/*
 * generated when a process opens a marked file
 */
struct acct_open {
	struct acct_common	ac_common;
	char			ac_path[PATH_MAX];
	int			ac_mode;	/* O_RDONLY | O_WRONLY */
	int			ac_errno;	/* 0 = success */
};

struct acct_rename {
	struct acct_common	ac_common;
	char			ac_path[PATH_MAX];
	char			ac_new[PATH_MAX];
	int			ac_errno;
};

struct acct_unlink {
	struct acct_common	ac_common;
	char			ac_path[PATH_MAX];
	int			ac_errno;
};

struct acct_close {
	struct acct_common	ac_common;
	char			ac_path[PATH_MAX];
};


#define	ACCT_ENA_FORK		(1 << 0)
#define	ACCT_ENA_EXEC		(1 << 1)
#define	ACCT_ENA_EXIT		(1 << 2)
#define	ACCT_ENA_OPEN		(1 << 3)
#define	ACCT_ENA_CLOSE		(1 << 4)
#define	ACCT_ENA_RENAME		(1 << 5)
#define	ACCT_ENA_UNLINK		(1 << 6)
#define	ACCT_ENA_ALL		ACCT_ENA_FORK | \
				ACCT_ENA_EXEC | \
				ACCT_ENA_EXIT | \
				ACCT_ENA_OPEN | \
				ACCT_ENA_CLOSE | \
				ACCT_ENA_RENAME | \
				ACCT_ENA_UNLINK

#define	ACCT_COND_READ		(1 << 0)
#define	ACCT_COND_WRITE		(1 << 1)
#define	ACCT_COND_SUCCESS	(1 << 2)
#define	ACCT_COND_FAILURE	(1 << 3)
#define	ACCT_COND_ALL		ACCT_COND_READ | \
				ACCT_COND_WRITE | \
				ACCT_COND_SUCCESS | \
				ACCT_COND_FAILURE

/*
 * Common structure for all of the acct ioctls.
 */
struct acct_ctl {
	uint32_t		acct_ena;		/* ACCT_ENA_* */
	uint32_t		acct_cond;		/* ACCT_COND_* */
	uint64_t		acct_fcount;		/* count of files */
	char			acct_path[PATH_MAX];
};

/*
 * Gets the global enabled auditing features.
 *
 * Output fields: acct_ena, acct_fcount
 */
#define	ACCT_IOC_STATUS		_IOR('j', 1, struct acct_ctl)

/*
 * Gets the set of enabled events and conditions applied to a particular
 * file (if any). Returns ENOENT if the file is not being tracked.
 *
 * Input fields: acct_path
 * Output fields: acct_ena, acct_cond
 */
#define	ACCT_IOC_FSTATUS	_IOWR('j', 2, struct acct_ctl)

/*
 * Enables audit features globally.
 *
 * Input fields: acct_ena (just the features to enable)
 * Output fields: acct_ena (all currently enabled features after applying)
 */
#define	ACCT_IOC_ENABLE		_IOWR('j', 3, struct acct_ctl)

/*
 * Disables audit features globally.
 *
 * Input fields: acct_ena (just the features to disable)
 * Output fields: acct_ena (all currently enabled features after applying)
 */
#define	ACCT_IOC_DISABLE	_IOWR('j', 4, struct acct_ctl)

/*
 * Starts tracking a file and generating audit events about it. The file must
 * exist at the specified path when this ioctl is called (if it doesn't, it
 * returns ENOENT).
 *
 * If the file specified is already being tracked, will add to the existing
 * events/conditions. Returns the final set of events/conditions for this file
 * in the struct.
 *
 * Input fields: acct_ena, acct_cond, acct_path
 * Output fields: acct_ena, acct_cond
 */
#define	ACCT_IOC_TRACK_FILE	_IOWR('j', 5, struct acct_ctl)

/*
 * Stops tracking a file for audit. The file must exist at the specified path.
 *
 * Removes all of the events and conditions specified from the tracking entry
 * for the file. If no events/conditions are left, the tracking should be
 * stopped altogether (and 0 returned in both acct_ena and acct_cond).
 * Otherwise, any remaining events/conditions should be returned.
 *
 * Use ACCT_ENA_ALL/ACCT_COND_ALL if you need to completely stop tracking of
 * a file.
 *
 * Input fields: acct_ena, acct_cond, acct_path
 * Output fields: acct_ena, acct_cond
 */
#define	ACCT_IOC_UNTRACK_FILE	_IOWR('j', 6, struct acct_ctl)



#ifdef _KERNEL

/* Process accounting hooks: leave these alone. */
void	acct_fork(struct process *);
void	acct_exec(struct process *);
void	acct_exit(struct process *);

/* File accounting hooks: you can change these if you want to. */
void	acct_open(struct process *, struct vnode *, int, int);
void	acct_rename(struct process *, struct vnode *, const char *, int);
void	acct_unlink(struct process *, struct vnode *, int);
void	acct_close(struct process *, struct vnode *);

#endif /* _KERNEL */

#endif /* __DEV_ACCT_H__ */
