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

#include <sys/types.h>
#include <sys/systm.h>

#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/un.h>

#include <sys/mbuf.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/task.h>

#include <sys/namei.h>
#include <sys/filedesc.h>
#include <sys/vnode.h>
#include <sys/pledge.h>

#include <sys/pfexecvar.h>
#include <sys/pfexec.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/syscallargs.h>

/*
 * Switch this on for debugging
 * #define PFEXDEBUG 0
 */

/*
 * Data required over taskq, and handling request
 */
struct sock_data {
	struct proc *p;

	int ident;
	struct taskq *tq;

	struct sys_pfexecve_args /* {
		syscallarg(const struct pfexecve_opts *) opts;
		syscallarg(const char *) path;
		syscallarg(char *const *) argp;
		syscallarg(char *const *) envp;
	}; */ *uap;

	struct socket *so;

	struct mbuf *req_m;
	struct pfexec_req *req;

	struct mbuf *resp_m;
	struct pfexec_resp *resp;

	struct task start;
	struct task end;

	struct ucred *orig_pcred;

	char **environment;

	int sys_pf_errno;
};

u_int req_mext_handler = 0;
struct rwlock req_lock = RWLOCK_INITIALIZER("destroy_req");

/*
 * Start a socket connection, if error fill in data
 */
void
listen_sock(void *arg)
{
#ifdef PFEXDEBUG
	printf("listen_sock -s\n");
#endif

	struct sock_data *data = (struct sock_data *)arg;
	struct socket *so = NULL;
	struct sockaddr *sa = NULL;
	struct sockaddr_un *su = NULL;
	struct mbuf *m = NULL, *mopt = NULL;
	int ret = 0, *slen = NULL, s;

	ret = socreate(AF_UNIX, &so, SOCK_SEQPACKET, 0);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: socreate error %d\n",
		    __func__, ret);
#endif
		goto close;
	}
#ifdef PFEXDEBUG
	printf("socket create\n");
#endif

	MGET(mopt, M_WAIT, MT_SOOPTS);
	mopt->m_len = sizeof(int);
	slen = mtod(mopt, int *);
	*slen = sizeof(struct pfexec_req);
	s = solock(so);
	ret = sosetopt(so, SOL_SOCKET, SO_SNDBUF, mopt);
	sounlock(so, s);
	m_freem(mopt);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: sosetopt error %d\n",
		    __func__, ret);
#endif
		goto close;
	}
#ifdef PFEXDEBUG
	printf("socket setopt(SNDBUF)\n");
#endif

	MGET(mopt, M_WAIT, MT_SOOPTS);
	mopt->m_len = sizeof(int);
	slen = mtod(mopt, int *);
	*slen = sizeof(struct pfexec_resp) + 32;
	s = solock(so);
	ret = sosetopt(so, SOL_SOCKET, SO_RCVBUF, mopt);
	sounlock(so, s);
	m_freem(mopt);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: sosetopt error %d\n",
		    __func__, ret);
#endif
		goto close;
	}
#ifdef PFEXDEBUG
	printf("socket setopt(RCVBUF)\n");
#endif // sockopt

	MGET(m, M_WAIT, MT_SONAME);
	m->m_len = sizeof(struct sockaddr_un);
	sa = mtod(m, struct sockaddr *);
	su = (struct sockaddr_un *)sa;
	su->sun_len = sizeof(struct sockaddr_un);
	su->sun_family = AF_UNIX;
	memcpy(su->sun_path, PFEXECD_SOCK, sizeof(su->sun_path));

	s = solock(so);
	ret = soconnect(so, m);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: soconnect error %d\n",
		    __func__, ret);
#endif
		goto bad;
	}
#ifdef PFEXDEBUG
	printf("socket connect\n");
#endif
	/*
	 * Wait for the connection to complete. Cribbed from the
	 * connect system call but with the wait timing out so
	 * that interruptible mounts don't hang here for a long time.
	 */
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0) {
		sosleep_nsec(so, &so->so_timeo, PF_UNIX, "pfexec-sock", \
		    SEC_TO_NSEC(2));
		if ((so->so_state & SS_ISCONNECTING) &&
		    so->so_error == 0) {
			so->so_state &= ~SS_ISCONNECTING;
			goto bad;
		}
	}
	if (so->so_error) {
		ret = so->so_error;
		so->so_error = 0;
		goto bad;
	}
	sounlock(so, s);
#ifdef PFEXDEBUG
	printf("socket connected\n");
#endif
	data->so = so;
	m_free(m);

close:
#ifdef PFEXDEBUG
	printf("listen_so -e\n");
#endif
	data->sys_pf_errno = !ret ? ret : ENOTCONN;
	return;
bad:
#ifdef PFEXDEBUG
	printf("%s: so connection error %d\n",
	    __func__, ret);
#endif
	sounlock(so, s);
	soclose(so, MSG_WAITALL);
	m_free(m);
	goto close;
}

/*
 * Callback function to free in the mbuf MEXT
 */
void
free_req(caddr_t addr, u_int size, void *arg)
{
	free(addr, M_TEMP, size);
}

/*
 * Fill in the data to send to the daemon
 */
void
craft_mbuf(void *arg)
{
#ifdef PFEXDEBUG
	printf("craft_mbuf -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;
	struct mbuf *m = NULL;
	struct pfexec_req *req = \
	    malloc(sizeof(struct pfexec_req), M_TEMP, M_WAITOK | M_ZERO);
	int reg = 0;

	if (req == NULL) {
		data->sys_pf_errno = ENOMEM;
		goto error;
	}

	MGETHDR(m, M_WAIT, MT_DATA);

	reg = 0;
	rw_enter_read(&req_lock);
	if (req_mext_handler == 0)
		req_mext_handler = mextfree_register(free_req);
	reg = req_mext_handler;
	rw_exit_read(&req_lock);

	MEXTADD(m, req, sizeof(struct pfexec_req), 0, reg, NULL);
	data->req_m = m;
	data->req = req;
error:
#ifdef PFEXDEBUG
	printf("craft_mbuf -e\n");
#endif // craft mub
	reg++;
}

/*
 * Send the request packet
 */
void
send_sock(void *arg)
{
#ifdef PFEXDEBUG
	printf("send_sock -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;
	struct socket *so = data->so;
	struct mbuf *m = data->req_m;
	int ret = 0;

	ret = sosend(so, NULL, NULL, m, NULL, MSG_WAITALL);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: sosend error %d\n",
		    __func__, ret);
#endif
		goto error;
	}

	data->req = NULL;
	data->req_m = NULL;
error:
	data->sys_pf_errno = ret;
#ifdef PFEXDEBUG
	printf("send_sock -e\n");
#endif // send_sock
}

/*
 * Receive the response packet
 */
void
rcv_sock(void *arg)
{
#ifdef PFEXDEBUG
	printf("rcv_sock -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;
	struct socket *so = data->so;
	struct pfexec_resp *resp = NULL;
	struct mbuf *from = NULL, *m = NULL;
	struct uio uio = {0};
	int ret = 0, flags = 0, len = 0, total = 0;

	total = len = uio.uio_resid = sizeof(struct pfexec_resp);
	flags = MSG_WAITALL;
	do {
		ret = soreceive(so, &from, &uio, &m, NULL, &flags, 0);
		if (ret /* && ret != EAGAIN */ || m == NULL) {
			if (m == NULL)
				ret = EPIPE;
#ifdef PFEXDEBUG
			printf("%s: soreceuive error %d\n",
			    __func__, ret);
#endif
			goto close;
		}
		len -= (total - uio.uio_resid);
	} while (len > 0);

	data->resp_m = m;
	resp = data->resp = \
	    malloc(sizeof(struct pfexec_resp), M_TEMP, M_WAITOK | M_ZERO);
	if (resp == NULL) {
		ret = ENOMEM;
		goto close;
	}

	m_copydata(m, 0, sizeof(struct pfexec_resp), data->resp);
#ifdef PFEXDEBUG
	printf("resp %p, resp.errno:%d\n", resp, resp->pfr_errno);
#endif
	m_freem(from);
close:
	data->sys_pf_errno = ret;
#ifdef PFEXDEBUG
	printf("rcv_sock -e\n");
#endif // rcv_sock
}

void
end_sock(void *arg)
{
#ifdef PFEXDEBUG
	printf("end_sock -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;

	soclose(data->so, MSG_WAITALL);

	m_freem(data->resp_m);
	data->resp_m = NULL;

	task_del(data->tq, &data->end);

	wakeup(&data->ident);
#ifdef PFEXDEBUG
	printf("end_sock -e\n");
#endif
}

/*
 * TaskQ handler of all managing socket traffic
 */
void
start_sock(void *arg)
{
#ifdef PFEXDEBUG
	printf("start_sock -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;

	listen_sock(data);
	if (data->sys_pf_errno != 0)
		goto bad;

	send_sock(data);
	if (data->sys_pf_errno != 0)
		goto error;

	rcv_sock(data);
	if (data->sys_pf_errno != 0)
		goto error;
error:
	task_del(data->tq, &data->start);

	task_add(data->tq, &data->end);
bad:
	wakeup(&data->ident);
#ifdef PFEXDEBUG
	printf("start_sock -e\n");
#endif
}

/*
 * Gather the data about this request and fill it in
 */
void
fill_mbuf(void *arg)
{
#ifdef PFEXDEBUG
	printf("fill_mbuf -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;
	const struct pfexecve_opts *opts = SCARG(data->uap, opts);
	struct process *proc = data->p->p_p;
	struct ucred *creds = data->p->p_p->ps_ucred;
	struct pfexec_req *req = data->req;
	int ret = 0;

#ifdef PFEXDEBUG
	printf("pid: %d\n", proc->ps_pid);
#endif

	/*	SYS_SPACE ARGS	*/
	memcpy(&req->pfr_pid, &proc->ps_pid, sizeof(req->pfr_pid));

	memcpy(&req->pfr_uid, &creds->cr_uid, sizeof(req->pfr_uid));
	memcpy(&req->pfr_gid, &creds->cr_gid, sizeof(req->pfr_gid));
	memcpy(&req->pfr_ngroups, &creds->cr_ngroups, \
	    sizeof(creds->cr_ngroups));
	memcpy(req->pfr_groups, creds->cr_groups, \
	    sizeof(req->pfr_groups));

	/*	USER_SPECE ARGS	*/

	size_t copied = 0;
	ret = copyin(&opts->pfo_flags, &data->req->pfr_req_flags, \
	    sizeof(data->req->pfr_req_flags));

	data->req->pfr_req_user[sizeof(data->req->pfr_req_user) - 1] = '\0';
	ret = copyinstr(opts->pfo_user, data->req->pfr_req_user, \
	    sizeof(data->req->pfr_req_user), &copied);

	data->req->pfr_path[sizeof(data->req->pfr_path) - 1] = '\0';
	ret = copyinstr(SCARG(data->uap, path), \
	    data->req->pfr_path, sizeof(data->req->pfr_path), &copied);

	copied = 0;
	int argc = 0;
	size_t max = ARG_MAX, len = 0;
	char *const *from = SCARG(data->uap, argp);

#ifdef PFEXDEBUG
		printf("args{");
#endif

	while (argc < 1024 && max > 0 && from != NULL && \
	    from[argc] != NULL) {
#ifdef PFEXDEBUG
		printf("%s ", from[argc]);
#endif
		data->req->pfr_argp[argc].pfa_offset = len;
		ret = copyinstr(from[argc], \
		    data->req->pfr_argarea + len, max - len, &copied);
		len += copied;
		max -= copied;
		data->req->pfr_argp[argc].pfa_len = copied - 1;
		argc++;
	}
	data->req->pfr_argarea[ARG_MAX - 1] = '\0';
#ifdef PFEXDEBUG
	printf("}\n");
#endif
	memcpy(&req->pfr_argc, &argc, sizeof(req->pfr_argc));

	copied = 0;
	int envc = 0;
	max = ARG_MAX;
	len = 0;
	from = SCARG(data->uap, envp);
#ifdef PFEXDEBUG
	printf("env{");
#endif
	while (envc < 1024 && max > 0 && from != NULL && \
	    from[envc] != NULL) {
#ifdef PFEXDEBUG
		printf("%s ", from[envc]);
#endif
		data->req->pfr_envp[envc].pfa_offset = len;
		ret = copyinstr(from[envc], \
		    data->req->pfr_envarea + len, max - len, &copied);
		len += copied;
		max -= copied;
		data->req->pfr_envp[envc].pfa_len = copied - 1;
		envc++;
	}
	data->req->pfr_envarea[ARG_MAX - 1] = '\0';
	memcpy(&req->pfr_envc, &envc, sizeof(req->pfr_envc));
#ifdef PFEXDEBUG
	printf("}\n");
#endif

#ifdef PFEXDEBUG
	printf("fill_mbuf -e\n");
#endif
}

/*
 * Show the response packet, useful to debug the response, switch on debug
 */
int
check_priv(struct sock_data *data)
{
	int ret = data->resp->pfr_errno;
	struct pfexec_resp *resp = data->resp;

#ifdef PFEXDEBUG
	printf("errno %d, flag %d, uid %d, gid %d, ngroups %d, groups {", \
	    resp->pfr_errno, resp->pfr_flags, resp->pfr_uid, \
	    resp->pfr_gid, resp->pfr_ngroups);
	for (int i = 0; i < resp->pfr_ngroups; i++) {
		printf("%d, ", resp->pfr_groups[i]);
	}
	printf("}\n");
#endif

#ifdef PFEXDEBUG
	printf("chroot: %s\n, envc %d\nenv {", resp->pfr_chroot, \
	    resp->pfr_envc);
#endif

	data->environment = malloc( \
	    sizeof(char *) * 1024, M_TEMP, M_WAITOK | M_ZERO);

	for (int i = 0; i < resp->pfr_envc; i++) {
#ifdef PFEXDEBUG
		printf("%s, ", resp->pfr_envarea + \
		    (resp->pfr_envp[i].pfa_offset));
#endif
		data->environment[i] = resp->pfr_envarea + \
		    (resp->pfr_envp[i].pfa_offset);
	}
#ifdef PFEXDEBUG
	printf("}\n");
#endif
	return (ret);
}

/*
 * Elivate this processes credentials
 */
void
pfexec_escilate(void *arg)
{
#ifdef PFEXDEBUG
	printf("pfexec_escilate -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;

	struct proc *p = data->p;
	struct process *pr = p->p_p;
	struct ucred *pruc, *newcred;
	int error = 0;
	int ngrp = (int)data->resp->pfr_ngroups;
	gid_t *groups = (gid_t *)data->resp->pfr_groups;
	uid_t old_uid = data->p->p_p->ps_ucred->cr_uid;

	if (ngrp > NGROUPS_MAX || ngrp < 0) {
		error = EINVAL;
		goto error;
	}
	newcred = crget();
	pruc = pr->ps_ucred;
	crset(newcred, pruc);

	newcred->cr_svuid = newcred->cr_ruid = newcred->cr_uid = \
	    data->resp->pfr_uid;
	newcred->cr_svgid = newcred->cr_rgid = newcred->cr_gid = \
	    data->resp->pfr_gid;

	newcred->cr_ngroups = ngrp;
	memset(newcred->cr_groups, 0, ngrp * sizeof(gid_t));
	memcpy(newcred->cr_groups, groups, ngrp * sizeof(gid_t));

	pr->ps_ucred = newcred;
	atomic_setbits_int(&p->p_p->ps_flags, PS_SUGID);

	chgproccnt(old_uid, -1);
	chgproccnt(data->resp->pfr_uid, 1);

	crfree(pruc);
	refreshcreds(p);
error:
	data->sys_pf_errno = error;

#ifdef PFEXDEBUG
	printf("pfexec_escilate -e\n");
#endif
}

/*
 * Free temporary buffers after escilation
 */
void
pfexec_descilate(void *arg)
{
#ifdef PFEXDEBUG
	printf("pfexec_descilate -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;

	free(data->environment, M_TEMP, sizeof(char *) * 1024);
#ifdef PFEXDEBUG
	printf("pfexec_descilate -e\n");
#endif // pfexec_desk
}

/*
 * Make a dynamic copy of path for NDINIT
 */
static char *
pfex_path_copy(char *path)
{
	char *my_path = (char *)malloc( \
	    sizeof(char) * PATH_MAX, M_DEVBUF, M_WAITOK | M_ZERO);

	strlcpy(my_path, path, PATH_MAX);

	return my_path;
}

/*
 * Common routine for chroot and chdir.
 */
static int
change_dir(struct nameidata *ndp, struct proc *p)
{
	struct vnode *vp;
	int error;

	if ((error = namei(ndp)) != 0)
		return (error);
	vp = ndp->ni_vp;
	if (vp->v_type != VDIR)
		error = ENOTDIR;
	else
		error = VOP_ACCESS(vp, VEXEC, p->p_ucred, p);
	if (error)
		vput(vp);
	else
		VOP_UNLOCK(vp);
	return (error);
}

/*
 * Change current working directory (``.'').
 */
int
apply_chdir(struct proc *p, char *path)
{
#ifdef PFEXDEBUG
	printf("pfexec_chdir -s\n");
#endif
	struct filedesc *fdp = p->p_fd;
	struct vnode *old_cdir;
	int error = 0;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_SYSSPACE,
	    path, p);
	nd.ni_pledge = PLEDGE_RPATH;
	nd.ni_unveil = UNVEIL_READ;
	if ((error = change_dir(&nd, p)) != 0)
		goto error;
	p->p_p->ps_uvpcwd = nd.ni_unveil_match;
	old_cdir = fdp->fd_cdir;
	fdp->fd_cdir = nd.ni_vp;
	vrele(old_cdir);

#ifdef PFEXDEBUG
	printf("pfexec_chdir -s\n");
#endif
error:
	return (error);
}

/*
 * Chroot
 */
int
apply_chroot(struct proc *p, char *path)
{
#ifdef PFEXDEBUG
	printf("pfexec_chroot -s\n");
#endif
	struct filedesc *fdp = p->p_fd;
	struct vnode *old_cdir, *old_rdir;
	int error;
	struct nameidata nd;

	NDINIT(&nd, LOOKUP, FOLLOW | LOCKLEAF, UIO_SYSSPACE,
	    path, p);
	if ((error = change_dir(&nd, p)) != 0)
		goto error;
	if (fdp->fd_rdir != NULL) {
		/*
		 * A chroot() done inside a changed root environment does
		 * an automatic chdir to avoid the out-of-tree experience.
		 */
		vref(nd.ni_vp);
		old_rdir = fdp->fd_rdir;
		old_cdir = fdp->fd_cdir;
		fdp->fd_rdir = fdp->fd_cdir = nd.ni_vp;
		vrele(old_rdir);
		vrele(old_cdir);
	} else
		fdp->fd_rdir = nd.ni_vp;

error:
#ifdef PFEXDEBUG
	printf("pfexec_chroot -e\n");
#endif
	return (error);
}

/*
 * Handler before actual call to sys_exec
 */
void
pfexec_exec(void *arg)
{
#ifdef PFEXDEBUG
	printf("pfexec_exec -s\n");
#endif
	struct sock_data *data = (struct sock_data *)arg;
	struct proc *p = data->p;
	int ret = 0;

	if (check_priv(data) != 0)
		goto denied;

	pfexec_escilate(arg);

	register_t retval = 0;
	struct sys_execve_args args = {0};

	SCARG(&args, path) = SCARG(data->uap, path);
	SCARG(&args, argp) = SCARG(data->uap, argp);
	SCARG(&args, kenvp) = data->environment;
	SCARG(&args, envp) = NULL;
#ifdef PFEXDEBUG
	printf("path: %s, argp: %p\n", \
	    SCARG(&args, path), SCARG(&args, argp));
#endif
	ret = sys_execve(p, &args, &retval);
	if (ret) {
#ifdef PFEXDEBUG
		printf("%s: sys_execve error %d\n",
		    __func__, ret);
#endif
	}

	if (data->resp->pfr_flags & PFRESP_CHROOT) {
		ret = apply_chroot(data->p, \
		    pfex_path_copy(data->resp->pfr_chroot));
		if (ret) {
#ifdef PFEXDEBUG
			printf("%s: apply_chroot %d\n",
			    __func__, ret);
#endif
		ret = apply_chroot(data->p, \
		    pfex_path_copy("/var/empty"));
		}
//		ret = apply_chroot(data->p, \
//		    pfex_path_copy(data->resp->pfr_chroot));
		if (ret) {
#ifdef PFEXDEBUG
			printf("%s: apply_chrdir %d\n",
			    __func__, ret);
#endif
		}
	}
	pfexec_descilate(arg);

denied:
#ifdef PFEXDEBUG
	printf("pfexec_exec -e\n");
#endif // pfexec_exec
	ret++;
}

/*
 * Validate the integrity of the response packet
 */
static int
process_response(struct pfexec_resp *resp)
{
	uint i;

	/* Check for correctly formed respuest. */
	if (resp->pfr_ngroups >= NGROUPS_MAX)
		return (EINVAL);
	if (resp->pfr_envc >= 1024)
		return (EINVAL);

	/*
	 * Validate all the argument and env var references before we try to
	 * use any of them.
	 */
	for (i = 0; i < resp->pfr_envc; ++i) {
		const struct pfexec_arg *a = &resp->pfr_envp[i];
		if (a->pfa_offset >= ARG_MAX)
			return (EINVAL);
		if (a->pfa_len >= ARG_MAX)
			return (EINVAL);
		if (a->pfa_offset + a->pfa_len >= ARG_MAX)
			return (EINVAL);
	}
	return (resp->pfr_errno);
}

/*
 * Check the arguments
 */
int
validate_args(void *v)
{
	struct sys_pfexecve_args /* {
		syscallarg(const struct pfexecve_opts *) opts;
		syscallarg(const char *) path;
		syscallarg(char *const *) argp;
		syscallarg(char *const *) envp;
	} */ *uap = v;
	if (SCARG(uap, opts) == NULL || SCARG(uap, path) == NULL || \
	    SCARG(uap, argp) == NULL || SCARG(uap, envp) == NULL)
		return EINVAL;
	return (0);
}

/*
 * Actual pfexec syscall handler
 */
int
sys_pfexecve(struct proc *p, void *v, register_t *retval)
{
#ifdef PFEXDEBUG
	printf("sys_pfexecve -s\n");
#endif
	struct sock_data data = {0};
	int ret = 0;

	ret = validate_args(v);
	if (ret)
		goto worse;

	data.p = p;

	data.tq = taskq_create("pfexec_tq", 1, IPL_SOFTNET, 0);
	if (data.tq == NULL) {
		data.sys_pf_errno = ENOMEM;
#ifdef PFEXDEBUG
		printf("%s: taskq_create error %d\n",
		    __func__, data.sys_pf_errno);
#endif
		goto worse;
	}

	data.uap = v;

	craft_mbuf(&data);
	if (data.sys_pf_errno != 0) {
		goto bad;
	}

	fill_mbuf(&data);

	task_set(&data.start, start_sock, &data);
	task_set(&data.end, end_sock, &data);

	task_add(data.tq, &data.start);

	/* Wait for connection establish and deamon to respond */
	ret = tsleep_nsec(&data.ident, IPL_SOFTNET | PCATCH, \
	    "SEND_NULL", INFSLP);
	if (ret) {
		data.sys_pf_errno = ret;
#ifdef PFEXDEBUG
		printf("%s: tlseep error %d\n",
		    __func__, ret);
#endif
		goto bad;
	}

	/* No errors then check the resp packet */
	if (ret == 0 && data.sys_pf_errno == 0)
		ret = process_response(data.resp);
	ret = data.sys_pf_errno ? data.sys_pf_errno : ret;
	if (ret != 0)
		goto bad;

	/* Check the resp then previlage esclate */
	pfexec_exec(&data);
	ret = data.sys_pf_errno;
bad:
	free(data.resp, M_TEMP, sizeof(struct pfexec_resp));

	taskq_destroy(data.tq);
#ifdef PFEXDEBUG
	printf("in debug mode\n");
#endif
worse:
#ifdef PFEXDEBUG
	printf("sys_pfexecve %d-e\n", data.sys_pf_errno);
#endif
	return (ret);
}
