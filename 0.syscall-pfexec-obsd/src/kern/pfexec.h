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

#if !defined(_SYS_PFEXEC_H_)
#define	_SYS_PFEXEC_H_

#include <sys/types.h>
#include <sys/syslimits.h>

enum pfexecve_flags {
	/* don't prompt for pws etc */
	PFEXECVE_NOPROMPT	= (1 << 0),
	/* request specific user (not root): pfo_user is valid */
	PFEXECVE_USER		= (1 << 1),

	PFEXECVE_ALL_FLAGS	= PFEXECVE_NOPROMPT |
				  PFEXECVE_USER
};

/* Additonal pfexec-specific options to pfexecve() etc */
struct pfexecve_opts {
	uint32_t	 pfo_flags;		/* see enum pfexecve_flags */

	/* valid if PFEXECVE_USER is set */
	char		 pfo_user[LOGIN_NAME_MAX];
};

int      pfexecve(const struct pfexecve_opts *, const char *, char *const *,
            char *const *);
int	 pfexecvp(const struct pfexecve_opts *, const char *, char *const *);
int	 pfexecvpe(const struct pfexecve_opts *, const char *, char *const *,
	    char *const *);

#endif /* !defined(_SYS_PFEXEC_H_) */
