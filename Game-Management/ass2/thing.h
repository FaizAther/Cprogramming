//	Mohammad Faiz ATHER === UQ 2020
//
//	thing.h
//	an ADT for a primitive char *Thing
//
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifndef THING_H_
#define THING_H_

typedef char *Thing;


static inline size_t
thing_size (Thing t)
{
	return (strlen(t)+1);
}

static inline Thing
thing_null (void)
{
	return NULL;
}

static inline Thing
thing_clean (Thing t)
{
	if (t == NULL) return thing_null();
	char *t_c = (char *)malloc ( sizeof (char) * thing_size (t) );
	int str_l = thing_size (t);
	int f = 0;
	int j = 0;
	for (int i = 0; i < str_l; i++)
	{
		if (t[i] == ' ')
		{
			for (j = i+1; t[j] == ' ' && j < str_l; j++)
			{
				// get to the non space
			}
			// j now has skipped all white space
			if (i != j-1) 
			{
				t_c[f] = t[i]; f++;
				t_c[f] = t[j]; f++;
				i = j;
			} else {
				t_c[f] = t[i]; f++;
			}
		} else {
			t_c[f] = t[i]; f++;
		}
	}
	if (t_c[f-2] == ' ') f--;
	t_c[f-1] = '\0';
	if (t_c[0] == ' ') {
		for (int t = 0; t < f; t++) {
			t_c[t] = t_c[t+1];
		}
	}
	return realloc(t_c, thing_size (t_c));
}

static inline Thing
thing_copy (Thing t)
{
	//sprintf (t_c, "%s", t);
	return thing_clean(t);
}

static inline void
thing_destroy (Thing t)
{
	free (t);
}

static inline Thing
thing_add (Thing src, Thing addon, char delem)
{
	int done = thing_size (src) + thing_size (addon);
	src = realloc (src, done);
	char s[2] = {delem, '\0'};
	strcat (src, s);
	return strcat (src, addon);
}

static inline void
thing_replace (Thing src, char from, char to)
{
	int len = thing_size (src);
	for (int i = 0; i < len; i++)
	{
		if (src[i]==from)
			src[i] = to;
	}
}

#define Thing Thing

#endif
