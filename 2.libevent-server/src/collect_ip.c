/*
 *
 * Copyright 2020 The University of Queensland
 * Author: Mohammad Faiz Ather <m.ather@uq.edu.au>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/sysctl.h>

#include <netinet/in.h>
#include <netinet/ip_var.h>

#include "metrics.h"
#include "log.h"

struct ip_modpriv {
	struct ipstat status;
	
	struct metric *ip_total;

	struct metric *ip_delivered, *ip_delivered_ops;
	
	struct metric *ip_localout, *ip_localout_ops;
	
	struct metric *ip_inswcsum;
	
	struct metric *ip_outswcsum;
};

struct metric_ops ip_metric_ops = {
	.mo_collect = NULL,
	.mo_free = NULL
};

static void
ip_register(struct registry *r, void **modpriv)
{
	struct ip_modpriv *priv;

	priv = calloc(1, sizeof(struct ip_modpriv));
	if (priv == NULL) {
		tserr(EXIT_MEMORY, "malloc");
	}
	*modpriv = priv;

	priv->ip_total = metric_new(r, "ip_total",
			"total packets received by ip",
			METRIC_GAUGE, METRIC_VAL_UINT32, NULL, &ip_metric_ops,
			NULL);

	priv->ip_delivered = metric_new(r, "ip_delivered",
			"packets sent for this host",
			METRIC_GAUGE, METRIC_VAL_UINT64, NULL, &ip_metric_ops,
			NULL);
	priv->ip_delivered_ops = metric_new(r, "ip_delivered_ops",
			"total packets sent for this host dropped for various reasons",
			METRIC_GAUGE, METRIC_VAL_UINT64, NULL, &ip_metric_ops,
			metric_label_new("reason", METRIC_VAL_STRING), NULL);

	priv->ip_localout = metric_new(r, "ip_localout",
			"packets sent to this host",
			METRIC_GAUGE, METRIC_VAL_UINT32, NULL, &ip_metric_ops,
			NULL);
	priv->ip_localout_ops = metric_new(r, "ip_localout_ops",
			"total packets sent to this host dropped for various reasons",
			METRIC_GAUGE, METRIC_VAL_UINT64, NULL, &ip_metric_ops,
			metric_label_new("reason", METRIC_VAL_STRING), NULL);

	priv->ip_inswcsum = metric_new(r, "ip_inswcsum",
			"input datagram software-checksummed",
			METRIC_GAUGE, METRIC_VAL_UINT32, NULL, &ip_metric_ops,
			NULL);

	priv->ip_outswcsum = metric_new(r, "ip_outswcsum",
			"output datagram software-checksummed",
			METRIC_GAUGE, METRIC_VAL_UINT64, NULL, &ip_metric_ops,
			NULL);
}

static int
ip_collect(void *modpriv)
{
	struct ip_modpriv *priv = modpriv;
	int mib[] = { CTL_NET, PF_INET, IPPROTO_IP, IPCTL_STATS };
	size_t len = sizeof(priv->status);

	if (sysctl(mib, sizeof(mib) / sizeof(mib[0]),
			&priv->status, &len, NULL, 0) == -1) {
		if (errno != ENOPROTOOPT)
			tslog("failed to get ip stats: %s", strerror(errno));
		return (0);
	}

	metric_update(priv->ip_total, priv->status.ips_total);

	metric_update(priv->ip_delivered, priv->status.ips_delivered);
	metric_update(priv->ip_delivered_ops, "badsum",
		priv->status.ips_badsum);
	metric_update(priv->ip_delivered_ops, "toosmall",
		priv->status.ips_toosmall);
	metric_update(priv->ip_delivered_ops, "tooshort",
		priv->status.ips_tooshort);
	metric_update(priv->ip_delivered_ops, "badhlen",
		priv->status.ips_badhlen);
	metric_update(priv->ip_delivered_ops, "badlen",
		priv->status.ips_badlen);
	metric_update(priv->ip_delivered_ops, "badoptions",
		priv->status.ips_badoptions);
	metric_update(priv->ip_delivered_ops, "badvers",
		priv->status.ips_badvers);
	metric_update(priv->ip_delivered_ops, "fragments",
		priv->status.ips_fragments);
	metric_update(priv->ip_delivered_ops, "fragdropped",
		priv->status.ips_fragdropped);
	metric_update(priv->ip_delivered_ops, "badfrags",
		priv->status.ips_badfrags);
	metric_update(priv->ip_delivered_ops, "fragtimeout",
		priv->status.ips_fragtimeout);
	metric_update(priv->ip_delivered_ops, "reassembled",
		priv->status.ips_reassembled);
	metric_update(priv->ip_delivered_ops, "noproto",
		priv->status.ips_noproto);
	metric_update(priv->ip_delivered_ops, "forward",
		priv->status.ips_forward);
	metric_update(priv->ip_delivered_ops, "cantforward",
		priv->status.ips_cantforward);
	metric_update(priv->ip_delivered_ops, "redirectsent",
		priv->status.ips_redirectsent);

	metric_update(priv->ip_localout, priv->status.ips_localout);
	metric_update(priv->ip_localout_ops, "rawout",
		priv->status.ips_rawout);
	metric_update(priv->ip_localout_ops, "odropped",
		priv->status.ips_odropped);
	metric_update(priv->ip_localout_ops, "noroute",
		priv->status.ips_noroute);
	metric_update(priv->ip_localout_ops, "fragmented",
		priv->status.ips_fragmented);
	metric_update(priv->ip_localout_ops, "ofragments",
		priv->status.ips_ofragments);
	metric_update(priv->ip_localout_ops, "cantfrag",
		priv->status.ips_cantfrag);
	metric_update(priv->ip_localout_ops, "rcvmemdrop",
		priv->status.ips_rcvmemdrop);
	metric_update(priv->ip_localout_ops, "toolong",
		priv->status.ips_toolong);
	metric_update(priv->ip_localout_ops, "nogif",
		priv->status.ips_nogif);
	metric_update(priv->ip_localout_ops, "badaddr",
		priv->status.ips_badaddr);
	metric_update(priv->ip_localout_ops, "notmember",
		priv->status.ips_notmember);
	metric_update(priv->ip_localout_ops, "wrongif",
		priv->status.ips_wrongif);

	metric_update(priv->ip_inswcsum, priv->status.ips_inswcsum);
	metric_update(priv->ip_outswcsum, priv->status.ips_outswcsum);

	metric_clear_old_values(priv->ip_total);
	metric_clear_old_values(priv->ip_delivered);
	metric_clear_old_values(priv->ip_localout);
	metric_clear_old_values(priv->ip_inswcsum);
	metric_clear_old_values(priv->ip_outswcsum);
	
	return (0);
}

static void
ip_free(void *modpriv)
{
	struct ip_modpriv *priv = modpriv;
	free(priv);
}

struct metrics_module_ops collect_ip_ops = {
	.mm_register = ip_register,
	.mm_collect = ip_collect,
	.mm_free = ip_free,
};