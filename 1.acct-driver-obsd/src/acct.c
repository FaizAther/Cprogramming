/*	$OpenBSD: acct.c,v 1.0 2021/10/10 07:59:38 ather $	*/

/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/exec.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/tty.h>
#include <sys/resourcevar.h>
#include <sys/pool.h>
#include <sys/pledge.h>
#include <sys/queue.h>

#include <lib/libkern/libkern.h>

#include <dev/acct.h>

extern int hz;
extern int tick;

/* TRK TREE DEFS */
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/tree.h>


struct trkvnode {
	char *trk_name;
	u_char trk_ena;
	u_char trk_flags;
	struct vnode *trkvnode;
	RBT_ENTRY(tkrvnode) trk_rbt;
};

RBT_HEAD(trkvnode_rbt, trkvnode);

#define TRACK_MAX_VNODES	128
#define TRACK_MAX_NAMES		128

static inline int
trkvnode_compare(const struct trkvnode *n1, const struct trkvnode *n2);

RBT_PROTOTYPE(trkvnode_rbt, trkvnode, trk_rbt, trkvnode_compare);
RBT_GENERATE(trkvnode_rbt, trkvnode, trk_rbt, trkvnode_compare);

struct trkvnode *
trkvnode_new(struct vnode *vnode, char *path);

void
trkvnode_delete(struct trkvnode *vnode);

int
trktree_delete_vnodes(struct trkvnode_rbt *trk);

struct nameidata *
path_to_namei(struct proc *p, char *path, struct nameidata *ndp);

struct trkvnode *
trktree_add_vnode_unlocked(struct trkvnode_rbt *trk,
		struct vnode *vnode, char *path);

struct trkvnode *
trktree_add_vnode(struct trkvnode_rbt *trk,
		struct vnode *vnode, char *path);

struct trkvnode *
trktree_vnoderemove(struct trkvnode_rbt *trk, struct vnode *vnode);

struct trkvnode *
trktree_vnodelookup(struct trkvnode_rbt *trk, struct vnode *vnode);

struct trkvnode *
trktree_pathremove(struct proc *p, struct trkvnode_rbt *trk, char *path);

struct trkvnode *
trktree_pathlookup(struct proc *p, struct trkvnode_rbt *trk, char *path);

void
trktree_destroy(struct trkvnode_rbt *trk);

int
trktree_parsepermissions(struct trkvnode *n, u_char ena, u_char flags);

char *
trk_path_copy(char *path);

struct trkvnode *
trktree_add_path(struct proc *p,
		struct trkvnode_rbt *trk, char *path);

/* TRK TREE DEFS END */

TAILQ_HEAD(acct_ccb_list, acct_node) acct_list;
struct acct_node {
	TAILQ_ENTRY(acct_node)	acct_nodes;
	struct acct_common *acc;
} *ac_n;

struct acct_ctl driver_ctl = {0};

int ac_seq = 0;

struct	rwlock ac_trklock = RWLOCK_INITIALIZER("ac_trklock");

struct	rwlock ac_taillock = RWLOCK_INITIALIZER("ac_tailq");
struct	rwlock ac_seqlock = RWLOCK_INITIALIZER("ac_seq");

static char identifier_sleep = 0;

static void
ac_seq_reset(void)
{
	rw_enter_read(&ac_seqlock);
	ac_seq = 0;
	rw_exit_read(&ac_seqlock);
}

struct trkvnode_rbt f_tracker = {0};

void
acctattach()
{
	printf("acctattach\n");

	TAILQ_INIT(&acct_list);
	RBT_INIT(trkvnode_rbt, &f_tracker);

	return;
}

int
acctopen(dev_t dev, int flag, int mode, struct proc *p)
{
	printf("acctopen\n");
	/* There are no non-zero minor devices */
	if (minor(dev) != 0)
		return (ENXIO);

	printf("acctopen done\n");
	return (0);
}

int
acctread(dev_t dev, struct uio *uio, int i)
{
	printf("acctread\n");
	int error;
	size_t len;
	struct acct_common *v;
	struct acct_node *tail =  NULL;

	if (driver_ctl.acct_ena == 0 && driver_ctl.acct_fcount == 0)
		return (EINVAL);

	if (uio->uio_offset < 0)
		return (EINVAL);
	
	rw_enter_read(&ac_taillock);

	tail = TAILQ_LAST(&acct_list, acct_ccb_list);
	while (tail == NULL) {
		rw_exit_read(&ac_taillock);
		printf("i am sleeping cause tail is null\n");
		error = tsleep_nsec(&identifier_sleep,
				PCATCH, "tail_null", INFSLP);
		if (error != EWOULDBLOCK && error != 0)
			return error;

		rw_enter_read(&ac_taillock);
		tail = TAILQ_LAST(&acct_list, acct_ccb_list);
	}
	printf("tail: %p, ac_seq %d\n", tail, tail->acc->ac_seq);

	int sent = 0;
	while (uio->uio_resid > 0) {
		if (sent >= tail->acc->ac_len) 
			break;
		
		v = tail->acc + sent;
		len = tail->acc->ac_len - sent;

		if (len > uio->uio_resid)
			len = uio->uio_resid;
		
		if ((error = uiomove(v, len, uio)) != 0)
			return (error);
		sent += len;
	}

	free(tail->acc, M_DEVBUF, tail->acc->ac_len);
	tail->acc = NULL;
	TAILQ_REMOVE(&acct_list, tail, acct_nodes);
	free(tail, M_DEVBUF, sizeof(struct acct_node));

	rw_exit_read(&ac_taillock);

	printf("acctread-exit\n");
	return (0);
}

//static uint32_t ENA = (ACCT_ENA_FORK | ACCT_ENA_EXEC | ACCT_ENA_EXIT);

static uint32_t F_ENA = (ACCT_ENA_OPEN | ACCT_ENA_CLOSE \
		| ACCT_ENA_RENAME | ACCT_ENA_UNLINK);

int
acctioctl(dev_t dev, u_long cmd, caddr_t data, int flag, struct proc *proc)
{
	printf("acctioctl\n");
	struct acct_ctl *ctl = (struct acct_ctl *)data;
	struct trkvnode *node = NULL;
	int error = 0;

	switch (cmd) {
	case FIONREAD:
		break;
	case FIONBIO:
		break;
	case ACCT_IOC_STATUS:
		ctl->acct_ena = driver_ctl.acct_ena;
		ctl->acct_fcount = driver_ctl.acct_fcount;
		break;
	case ACCT_IOC_FSTATUS:
		node = trktree_pathlookup(proc, &f_tracker, ctl->acct_path);
		if (node != NULL) {
			ctl->acct_ena = node->trk_ena;
			//ctl->acct_ena &= (driver_ctl.acct_ena | F_ENA);
			ctl->acct_cond = node->trk_flags;
		}
		ctl->acct_fcount = driver_ctl.acct_fcount;
		break;
	case ACCT_IOC_ENABLE:
		driver_ctl.acct_ena |= ctl->acct_ena;
		ctl->acct_ena = driver_ctl.acct_ena;
		break;
	case ACCT_IOC_DISABLE:
		if (ctl->acct_ena & ACCT_ENA_ALL) {
			trktree_delete_vnodes(&f_tracker);
		}
		driver_ctl.acct_ena &= !ctl->acct_ena;
		ctl->acct_ena = driver_ctl.acct_ena;
		ctl->acct_fcount = driver_ctl.acct_fcount;
		break;
	case ACCT_IOC_TRACK_FILE:
		node = trktree_add_path(proc,
				&f_tracker, ctl->acct_path);
		if (node != NULL) {
			node->trk_ena |= (ctl->acct_ena & F_ENA);
			node->trk_flags |= ctl->acct_cond;
			ctl->acct_ena = node->trk_ena;
			ctl->acct_cond = node->trk_flags;
		} else {
			ctl->acct_ena = 0;
			ctl->acct_cond = 0;
			error = ENOENT;
		}
		ctl->acct_fcount = driver_ctl.acct_fcount;
		break;
	case ACCT_IOC_UNTRACK_FILE:
		ctl->acct_ena = driver_ctl.acct_ena;
		node = trktree_pathremove(proc, &f_tracker, ctl->acct_path);
		ctl->acct_fcount = driver_ctl.acct_fcount;
		ctl->acct_cond = 0;
		break;
	default:
		break;
	}
	return (error);
}

void
acct_void(struct acct_common *ac_comm, unsigned short ac_type,
		unsigned short ac_len, struct process *pr)
{
	struct timespec booted, elapsed, realstart, uptime;

	ac_comm->ac_len = ac_len;
	ac_comm->ac_type = ac_type;

	/* (1) Name of process */
	memcpy(ac_comm->ac_comm, pr->ps_comm, sizeof(char) * 16);

	/* (3) The elapsed time the command ran (and its starting time) */
	nanouptime(&uptime);
	nanoboottime(&booted);
	timespecadd(&booted, &pr->ps_start, &realstart);
	memcpy(&(ac_comm->ac_btime), &realstart, sizeof(struct timespec));
	timespecsub(&uptime, &pr->ps_start, &elapsed);
	memcpy(&(ac_comm->ac_etime), &elapsed, sizeof(struct timespec));
	
	/* (6) The PID, UID and GID of the process */
	ac_comm->ac_pid = pr->ps_pid;
	ac_comm->ac_uid = pr->ps_ucred->cr_ruid;
	ac_comm->ac_gid = pr->ps_ucred->cr_rgid;

	/* (7) The terminal from which the process was started */
	if ((pr->ps_flags & PS_CONTROLT) &&
	    pr->ps_pgrp->pg_session->s_ttyp)
		ac_comm->ac_tty = pr->ps_pgrp->pg_session->s_ttyp->t_dev;
	else
		ac_comm->ac_tty = NODEV;

	/* (8) The boolean flags tell how process terminated or misbehaved. */
	ac_comm->ac_flag = pr->ps_acflag;

	return; 
}

static struct acct_node *
make_node(void *ac_val)
{
	struct acct_node *node =
		(struct acct_node *)malloc(sizeof(struct acct_node),
				M_DEVBUF, M_WAITOK | M_ZERO);
	node->acc = ac_val;

	return (node);
}

static int
update_seq(void)
{
	int ret = 0;
	rw_enter_read(&ac_seqlock);
	ret = ac_seq;
	ac_seq += 1;
	rw_exit_read(&ac_seqlock);

	return ret;
}

static void
add_node(struct acct_node *node)
{
	int wake_him = 0;
	if (TAILQ_EMPTY(&acct_list))
		wake_him = 1;
	
	rw_enter_read(&ac_taillock);
	TAILQ_INSERT_HEAD(&acct_list, node, acct_nodes);
	node->acc->ac_seq = update_seq();
	printf("add_node seq: %d\n", node->acc->ac_seq);
	rw_exit_read(&ac_taillock);

	if (wake_him == 1) {
		printf("i am waking him up\n");
		wakeup(&identifier_sleep);
	}
	return;
}

void
acct_fork(struct process *pr)
{
	if (!(driver_ctl.acct_ena & ACCT_ENA_FORK))
		return;

	struct acct_fork *ac_fork =
		(struct acct_fork *)malloc(sizeof(struct acct_fork),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_fork == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_fork->ac_common),
		ACCT_MSG_FORK, sizeof(struct acct_fork),
		pr->ps_pptr);

	ac_fork->ac_cpid = pr->ps_pid;

	struct acct_node *node = make_node(ac_fork);

	add_node(node);

	return;
}

void
acct_exec(struct process *pr)
{
	if (!(driver_ctl.acct_ena & ACCT_ENA_EXEC))
		return;

	struct acct_exec *ac_exec =
		(struct acct_exec *)malloc(sizeof(struct acct_exec),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_exec == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_exec->ac_common),
		ACCT_MSG_EXEC, sizeof(struct acct_exec),
		pr);

	struct acct_node *node = make_node(ac_exec);

	add_node(node);

	return;
}

void
acct_exit(struct process *pr)
{
	if (!(driver_ctl.acct_ena & ACCT_ENA_EXIT))
		return;

	struct timespec st, tmp, ut;
	struct rusage *r;
	struct acct_exit *ac_exit =
		(struct acct_exit *)malloc(sizeof(struct acct_exit),
				M_DEVBUF, M_WAITOK | M_ZERO);
	int t;
	if (ac_exit == NULL) {
		update_seq();
		return;
	}


	acct_void(&(ac_exit->ac_common),
		ACCT_MSG_EXIT, sizeof(struct acct_exit),
		pr);

	struct proc *p = pr->ps_mainproc;

	/* (2) The amount of user and system time that was used */
	calctsru(&pr->ps_tu, &ut, &st, NULL);
	memcpy(&ac_exit->ac_utime, &ut, sizeof(struct timespec));
	memcpy(&ac_exit->ac_stime, &st, sizeof(struct timespec));

	/* (4) The average amount of memory used */
	r = &p->p_ru;
	timespecadd(&ut, &st, &tmp);
	t = tmp.tv_sec * hz + tmp.tv_nsec / (1000 * tick);
	if (t)
		ac_exit->ac_mem =
			(r->ru_ixrss + r->ru_idrss + r->ru_isrss) / t;
	else
		ac_exit->ac_mem = 0;

	/* (5) The number of disk I/O operations done */
	ac_exit->ac_io = r->ru_inblock + r->ru_oublock;

	struct acct_node *node = make_node(ac_exit);

	add_node(node);

	return;
}

void
acct_open(struct process *pr, struct vnode *vn, int i0, int i1)
{
//	printf("acct_open\n");

	struct trkvnode *n = trktree_vnodelookup(&f_tracker, vn);

	if (trktree_parsepermissions(n, ACCT_ENA_OPEN, (u_char)i0) != 0)
		return;

	struct acct_open *ac_open =
		(struct acct_open *)malloc(sizeof(struct acct_open),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_open == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_open->ac_common),
		ACCT_MSG_OPEN, sizeof(struct acct_open),
		pr);

	ac_open->ac_mode = i0;
	ac_open->ac_errno = i1;
	strlcpy(ac_open->ac_path, n->trk_name, PATH_MAX);

	struct acct_node *node = make_node(ac_open);
	add_node(node);
	
	printf("acct_open end\n");
	return;
}

void
acct_rename(struct process *pr, struct vnode *vn, const char *name, int e0)
{
	struct trkvnode *n = trktree_vnodelookup(&f_tracker, vn);

	if (trktree_parsepermissions(n, ACCT_ENA_RENAME, 0) != 0) {
		if (n != NULL)
			trktree_vnoderemove(&f_tracker, n->trkvnode);		
		return;
	}
	
	struct acct_rename *ac_rename =
		(struct acct_rename *)malloc(sizeof(struct acct_rename),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_rename == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_rename->ac_common),
		ACCT_MSG_RENAME, sizeof(struct acct_rename),
		pr);

	strlcpy(ac_rename->ac_path, n->trk_name, PATH_MAX);
	memcpy(ac_rename->ac_new, name, PATH_MAX);
	ac_rename->ac_errno = e0;

	struct acct_node *node = make_node(ac_rename);
	add_node(node);

	trktree_vnoderemove(&f_tracker, n->trkvnode);
	printf("acct_rename end\n");
	return;
}
void
acct_unlink(struct process *pr, struct vnode *vn, int e0)
{
	struct trkvnode *n = trktree_vnodelookup(&f_tracker, vn);

	if (trktree_parsepermissions(n, ACCT_ENA_UNLINK, 0) != 0) {
		if (n != NULL)
			trktree_vnoderemove(&f_tracker, n->trkvnode);
		return;
	}

	struct acct_unlink *ac_unlink =
		(struct acct_unlink *)malloc(sizeof(struct acct_unlink),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_unlink == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_unlink->ac_common),
		ACCT_MSG_UNLINK, sizeof(struct acct_unlink),
		pr);

	strlcpy(ac_unlink->ac_path, n->trk_name, PATH_MAX);
	ac_unlink->ac_errno = e0;

	struct acct_node *node = make_node(ac_unlink);
	add_node(node);

	trktree_vnoderemove(&f_tracker, n->trkvnode);
	printf("acct_unlink end\n");
	return;
}

void
acct_close(struct process *pr, struct vnode *vn)
{
	printf("acct_close\n");
	struct trkvnode *n = trktree_vnodelookup(&f_tracker, vn);
	if (trktree_parsepermissions(n, ACCT_ENA_CLOSE, 0) != 0)
		return;

	struct acct_close *ac_close =
		(struct acct_close *)malloc(sizeof(struct acct_close),
				M_DEVBUF, M_WAITOK | M_ZERO);
	if (ac_close == NULL) {
		update_seq();
		return;
	}

	acct_void(&(ac_close->ac_common),
		ACCT_MSG_CLOSE, sizeof(struct acct_close),
		pr);

	strlcpy(ac_close->ac_path, n->trk_name, PATH_MAX);

	struct acct_node *node = make_node(ac_close);
	add_node(node);

	printf("acct_close end\n");
	return;
}

int
acctclose(dev_t dev, int flag, int mode, struct proc *p)
{
	printf("acctclose\n");
	rw_enter_read(&ac_taillock);	
	struct acct_node *np = NULL;
	while ((np = TAILQ_FIRST(&acct_list))) {
		free(np->acc, M_DEVBUF, np->acc->ac_len);
		TAILQ_REMOVE(&acct_list, np, acct_nodes);
		free(np, M_DEVBUF, sizeof(struct acct_node));
	}
	rw_exit_read(&ac_taillock);
	ac_seq_reset();

	printf("acctclose end\n");
	return (0);
}

int
acctkqfilter(dev_t dev, struct knote *p)
{
	return (EOPNOTSUPP);
}

int
acctpoll(dev_t dev, int flag, struct proc *p)
{
	return (POLLERR);
}

/*
 * Unsupported ioctl function.
 */
int
acctwrite(dev_t dev, struct uio *uio, int i)
{
	return (EOPNOTSUPP);
}

static inline int
trkvnode_compare(const struct trkvnode *n1, const struct trkvnode *n2)
{
//	printf("trkcompare %p %p\n", n1->trkvnode, n2->trkvnode);
	return n1->trkvnode - n2->trkvnode;
}

struct trkvnode *
trkvnode_new(struct vnode *vnode, char *path)
{
	if (vnode == NULL)
		return NULL;

	struct trkvnode *ret = malloc(sizeof(struct trkvnode),
			M_DEVBUF, M_WAITOK|M_ZERO);
	ret->trkvnode = vnode;
	ret->trk_name = trk_path_copy(path);
	return ret;
}

void
trkvnode_delete(struct trkvnode *trk_vnode)
{
	if (trk_vnode == NULL)
		return;

	free(trk_vnode->trk_name, M_DEVBUF, sizeof(char) * PATH_MAX);
	free(trk_vnode, M_DEVBUF, sizeof(struct trkvnode));
}

int
trktree_delete_vnodes(struct trkvnode_rbt *trk)
{
	struct trkvnode *trkv, *next;
	int ret = 0;

	rw_enter_write(&ac_trklock);
	RBT_FOREACH_SAFE(trkv, trkvnode_rbt, trk, next) {
		//vrele(trkv->trkvnode);
		RBT_REMOVE(trkvnode_rbt, trk, trkv);
		trkvnode_delete(trkv);
		ret++;
	}
	rw_exit_write(&ac_trklock);
	printf("deleted %d names\n", ret);
	return ret;
}

void
trktree_destroy(struct trkvnode_rbt *trk)
{
	trktree_delete_vnodes(trk);
	trk = NULL;
}

struct trkvnode *
trktree_add_vnode_unlocked(struct trkvnode_rbt *trk,
		struct vnode *trk_vnode, char *path)
{
	struct trkvnode *trkv, *old;

	trkv = trkvnode_new(trk_vnode, path);
	old = RBT_INSERT(trkvnode_rbt, trk, trkv);
	if (old != NULL) {
		/* Name already present. */
		trkvnode_delete(trkv);
		return old;
	} else
		(driver_ctl.acct_fcount)++;

	printf("added vnode %p underneath tkr %p\n", trk_vnode, trk);
	return trkv;
}

struct trkvnode *
trktree_add_vnode(struct trkvnode_rbt *trk,
		struct vnode *vnode, char *path)
{
	struct trkvnode *ret;

	if (vnode == NULL)
		return NULL;

	rw_enter_write(&ac_trklock);
	ret = trktree_add_vnode_unlocked(trk, vnode, path);
	rw_exit_write(&ac_trklock);
	return ret;
}

struct trkvnode *
trktree_vnoderemove(struct trkvnode_rbt *trk, struct vnode *vnode)
{
	printf("trk_vnoderemove\n");
	struct trkvnode trkv, *ret = NULL;

	rw_enter_read(&ac_trklock);

	printf("trk_vnoderemove: looking up vnode %p in tkr %p\n",
	    vnode, trk);
	trkv.trkvnode = vnode;
	ret = RBT_FIND(trkvnode_rbt, trk, &trkv);
	if (ret != NULL) {
		ret = RBT_REMOVE(trkvnode_rbt, trk, ret);
		trkvnode_delete(ret);
		(driver_ctl.acct_fcount)--;
	}
	else
		printf("did not find vnode\n");

	rw_exit_read(&ac_trklock);

	if (ret == NULL)
		printf("trk_vnoderemove: no match for vnode %p in trk %p\n",
		    vnode, trk);
	else
		printf("trk_vnoderemove: matched vnode %p in trk %p\n",
		    vnode, trk);
	printf("trk_vnoderemove exit %p\n", ret);
	return ret;
}

struct trkvnode *
trktree_vnodelookup(struct trkvnode_rbt *trk, struct vnode *vnode)
{
//	printf("trk_vnodelookup\n");

	struct trkvnode trkv, *ret = NULL;

	rw_enter_read(&ac_trklock);

//	printf("trk_vnodelookup: looking up vnode %p in tkr %p\n",
//	    vnode, trk);
	trkv.trkvnode = vnode;
	ret = RBT_FIND(trkvnode_rbt, trk, &trkv);

	rw_exit_read(&ac_trklock);

//	if (ret == NULL)
//		printf("trk_vnodelookup: no match for vnode %p in trk %p\n",
//		    vnode, trk);
//	else
//		printf("trk_vnodelookup: matched vnode %p in trk %p\n",
//		    vnode, trk);
	if (ret != NULL)
		printf("trk_vnodelookup exit %p\n", ret);
	return ret;
}

char *
trk_path_copy(char *path)
{
	char *my_path = (char *)malloc(
		sizeof(char) * PATH_MAX, M_DEVBUF, M_WAITOK | M_ZERO);

	int cpd = strlcpy(my_path, path, 49);
	if (cpd > 0)
		printf("success cpd: %s, src: %s\n", my_path, path);
	else
		printf("fail cpd: %s, src: %s\n", my_path, path);

	return my_path;
}

struct vnode *
path_to_vnode(struct proc *p, char *path)
{
	struct nameidata nd = {0};
	char *my_path = trk_path_copy(path);

	NDINIT(&nd, LOOKUP,FOLLOW|LOCKLEAF|LOCKPARENT,
			UIO_SYSSPACE, my_path, p);

	int error = namei(&nd);
	if (error == 0 && nd.ni_dvp != NULL)
		vput(nd.ni_dvp);
	if (error == 0 && nd.ni_vp != NULL)
		vput(nd.ni_vp);

	free(my_path, M_DEVBUF, sizeof(char) * PATH_MAX);
	return error == 0 ? nd.ni_vp : NULL;
}

struct trkvnode *
trktree_pathremove(struct proc *p, struct trkvnode_rbt *trk, char *path)
{
	printf("trktree_pathremove\n");
	struct vnode *v = path_to_vnode(p, path);
	struct trkvnode *ret = NULL;
	
	if (v == NULL)
		return NULL;

	ret = trktree_vnoderemove(trk, v);

	printf("trktree_path exit %p vnode:%p\n", ret, v);
//	if (v != NULL)
//		vput(v);
	return ret;
}

struct trkvnode *
trktree_pathlookup(struct proc *p, struct trkvnode_rbt *trk, char *path)
{
	printf("trktree_path\n");
	struct vnode *v = path_to_vnode(p, path);
	struct trkvnode *ret = NULL;

	if (v == NULL)
		return NULL;

	ret = trktree_vnodelookup(trk, v);

	printf("trktree_path exit %p vnode:%p\n", ret, v);
//	if (v != NULL)
//		vput(v);
	return ret;
}

struct trkvnode *
trktree_add_path(struct proc *p,
		struct trkvnode_rbt *trk, char *path)
{
	printf("trk_add_path\n");
	struct trkvnode *ret;
	struct vnode *v = path_to_vnode(p, path);

	ret = trktree_add_vnode(trk, v, path);

	printf("added path %s beneath %p vnode %p,",
	    path, trk, v);

	printf("trk_add_path exit\n");
	return ret;
}

int
trktree_parsepermissions(struct trkvnode *n, u_char ena, u_char flags)
{
	if (n == NULL)
		return 1;
	printf("driverctl.ena %d trk_ena %dtrk_con %d\nena %d, flags %d\n",
			driver_ctl.acct_ena,
			n->trk_ena, n->trk_flags, ena, flags);

	if (flags == 0)
		flags = n->trk_flags;

	if ((driver_ctl.acct_ena & ena) && (n->trk_ena & ena) && \
		((flags == 0) || (n->trk_flags & flags)))
		return 0;

	return 2;
}
