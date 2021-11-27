/* $OpenBSD: env.c,v 1.10 2019/07/07 19:21:28 tedu Exp $ */
/*
 * Copyright (c) 2016 Ted Unangst <tedu@openbsd.org>
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
#include <sys/tree.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

#include "pfexecd.h"

struct envnode {
	RB_ENTRY(envnode) node;
	const char *key;
	const char *value;
};

struct env {
	RB_HEAD(envtree, envnode) root;
	u_int count;
};

static char *
find_in_env(const char *name, int len, int *offset, const char **my_env)
{
	int i;
	const char *np;
	char **p, *cp;

	char **my_dup = (char **)my_env;

	if (name == NULL || my_env == NULL)
		return (NULL);
	for (p = my_dup + *offset; (cp = *p) != NULL; ++p) {
		for (np = name, i = len; i && *cp; i--)
			if (*cp++ != *np++)
				break;
		if (i == 0 && *cp++ == '=') {
			*offset = p - my_dup;
			return (cp);
		}
	}
	return (NULL);
}

/*
 * get_my_env --
 *	Returns ptr to value associated with name, if any, else NULL.
 */
static char *
get_my_env(const char *name, const char **my_env)
{
	int offset = 0;
	const char *np;

	for (np = name; *np && *np != '='; ++np)
		;
	return (find_in_env(name, (int)(np - name), &offset, my_env));
}

static void fillenv(struct env *env, const char **envlist, const char **my_env);

static int
envcmp(struct envnode *a, struct envnode *b)
{
	return strcmp(a->key, b->key);
}
RB_GENERATE_STATIC(envtree, envnode, node, envcmp);

static struct envnode *
createnode(const char *key, const char *value)
{
	struct envnode *node;

	node = malloc(sizeof(*node));
	if (!node)
		err(1, NULL);
	node->key = strdup(key);
	node->value = strdup(value);
	if (!node->key || !node->value)
		err(1, NULL);
	return node;
}

static void
freenode(struct envnode *node)
{
	free((char *)node->key);
	free((char *)node->value);
	free(node);
}

static void
addnode(struct env *env, const char *key, const char *value)
{
	struct envnode *node;

	node = createnode(key, value);
	RB_INSERT(envtree, &env->root, node);
	env->count++;
}

static struct env *
createenv(const struct rule *rule, char *logname, struct passwd *targpw, \
    const char **my_env)
{
	static const char *copyset[] = {
		"DISPLAY", "TERM",
		NULL
	};
	struct env *env;
	u_int i;
	char *safepath = "/bin:/sbin:/usr/bin:/usr/sbin:"
	    "/usr/local/bin:/usr/local/sbin";

	env = malloc(sizeof(*env));
	if (!env)
		err(1, NULL);
	RB_INIT(&env->root);
	env->count = 0;

	addnode(env, "PFEXEC_USER", rule->ident);
	addnode(env, "HOME", targpw->pw_dir);
	addnode(env, "LOGNAME", targpw->pw_name);
	addnode(env, "USER", targpw->pw_name);
	addnode(env, "SHELL", targpw->pw_shell);
	fillenv(env, copyset, my_env);

	if (rule->options & KEEPENV) {
		for (i = 0; my_env[i] != NULL; i++) {
			struct envnode *node;
			const char *e, *eq;
			size_t len;
			char name[1024];

			e = my_env[i];

			/* ignore invalid or overlong names */
			if ((eq = strchr(e, '=')) == NULL || eq == e)
				continue;
			len = eq - e;
			if (len > sizeof(name) - 1)
				continue;
			memcpy(name, e, len);
			name[len] = '\0';

			node = createnode(name, eq + 1);
			if (RB_INSERT(envtree, &env->root, node)) {
				/* ignore any later duplicates */
				freenode(node);
			} else {
				env->count++;
			}
		}
	}

	/* Add the path later if not added by KEEPENV */
	struct envnode *node = createnode("SHELL", safepath);
	if (RB_INSERT(envtree, &env->root, node)) {
		/* ignore any later duplicates */
		freenode(node);
	} else {
		env->count++;
	}

	return env;
}

static char **
flattenenv(struct env *env)
{
	char **envp;
	struct envnode *node, *tmp;
	u_int i;

	envp = reallocarray(NULL, env->count + 1, sizeof(char *));
	if (!envp)
		err(1, NULL);
	i = 0;

	RB_FOREACH_SAFE(node, envtree, &env->root, tmp) {
		if (asprintf(&envp[i], "%s=%s", node->key, node->value) == -1)
			err(1, NULL);
		RB_REMOVE(envtree, &env->root, node);
		env->count--;
		freenode(node);
		i++;
	}
	envp[i] = NULL;
	return envp;
}

static void
fillenv(struct env *env, const char **envlist, const char **my_env)
{
	struct envnode *node, key;
	const char *e, *eq;
	const char *val;
	char name[1024];
	u_int i;
	size_t len;

	for (i = 0; envlist[i]; i++) {
		e = envlist[i];

		/* parse out env name */
		if ((eq = strchr(e, '=')) == NULL)
			len = strlen(e);
		else
			len = eq - e;
		if (len > sizeof(name) - 1)
			continue;
		memcpy(name, e, len);
		name[len] = '\0';

		/* delete previous copies */
		key.key = name;
		if (*name == '-')
			key.key = name + 1;
		if ((node = RB_FIND(envtree, &env->root, &key))) {
			RB_REMOVE(envtree, &env->root, node);
			freenode(node);
			env->count--;
		}
		if (*name == '-')
			continue;

		/* assign value or inherit from my_env */
		if (eq) {
			val = eq + 1;
			if (*val == '$') {
				val = get_my_env(val + 1, my_env);
			}
		} else {
				val = get_my_env(name, my_env);
		}
		/* at last, we have something to insert */
		if (val) {
			node = createnode(name, val);
			RB_INSERT(envtree, &env->root, node);
			env->count++;
		}
	}
}

char **
prepenv(const struct rule *rule, char *logname, struct passwd *targpw, \
    const char **my_env)
{
	struct env *env;

	env = createenv(rule, logname, targpw, my_env);
	if (rule->envlist)
		fillenv(env, rule->envlist, my_env);

	return flattenenv(env);
}
