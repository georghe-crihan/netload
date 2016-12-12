/*-
 * Copyright (c) 1999 Assar Westerlund
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>

#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/queue.h>

#include "netload.h"

static struct _if_data ifdt[MAX_IFACES],
		       *id = &ifdt[0];
static struct callout_handle toh;
static struct timeval tv = {1, 0};

static int poll_timeout=0;

static __inline
int read_load(struct _if_data *id)
{
	struct ifnet *ifp;
	u_long c;

	(id->stat_counter)++;
	(id->stat_counter)&=(MAX_STAT-1);

/* /sys/net/rtsock.c @ sysctl_iflist; */
	TAILQ_FOREACH(ifp, &ifnet, if_link)
	{
		if( ( strcmp( id->if_name, ifp->if_name ) != 0 ) ||
		    ( id->if_unit != ifp->if_unit ) ||
		    !( ifp->if_flags & IFF_UP ) )
		  continue;
		id->if_packets.in = ifp->if_data.ifi_ipackets;
		id->if_packets.out = ifp->if_data.ifi_opackets;
		c = ifp->if_data.ifi_ibytes;
		if( c > id->if_total.in )
		  id->stat[id->stat_counter].in = c - id->if_total.in;
		id->if_total.in = c;
			
		c = ifp->if_data.ifi_obytes;
		if( c > id->if_total.out )
		  id->stat[id->stat_counter].out = c - id->if_total.out;
		id->if_total.out = c;
				
		return 1;
	}
	return 0;
}

static
void timer_handler(void *ifchain)
{
  if (!poll_timeout)
    return;
  callout_handle_init(&toh);
  untimeout(timer_handler, (void *) id, toh);
  toh=timeout(timer_handler, (void *) id, poll_timeout);
  read_load((struct _if_data *) ifchain);
}

struct netload_args {
	int cmd;
	int iface;
	struct _if_data * id;
};

/*
 * The function for implementing the syscall.
 */

static int
netload (struct proc *p, struct netload_args *uap)
{
int rc = 0;

	if (uap->id == NULL)
		return EINVAL;

	switch(uap->cmd) {
	case LOAD_READ :
		rc = copyout(&ifdt[uap->iface], uap->id, sizeof(struct _if_data));
		break;
	case LOAD_SET :
		poll_timeout=0;
		callout_handle_init(&toh);
		untimeout(timer_handler, (void *) id, toh);
		rc=copyin( uap->id, &ifdt[uap->iface], sizeof(struct _if_data));
		poll_timeout = tvtohz(&tv);
		toh=timeout(timer_handler, (void *) id, poll_timeout);
		break;
	default :
		rc=EINVAL;
	}
	return rc;
}

/*
 * The `sysent' for the new syscall
 */

static struct sysent netload_sysent = {
	3,			/* sy_narg */
	netload			/* sy_call */
};

/*
 * The offset in sysent where the syscall is allocated.
 */

static int offset = NO_SYSCALL;

/*
 * The function called at load/unload.
 */

static int
load (struct module *module, int cmd, void *arg)
{
	int error = 0;

	switch (cmd) {
	case MOD_LOAD :
		memset(ifdt, 0, sizeof(struct _if_data) * MAX_IFACES);
		break;
	case MOD_UNLOAD :
		poll_timeout=0;
		callout_handle_init(&toh);
		untimeout(timer_handler, (void *) id, toh);
/*		printf ("syscall unloaded from %d\n", offset);
*/
		break;
	default :
		error = EINVAL;
		break;
	}
	return error;
}

/* offset is the syscall slot # */
SYSCALL_MODULE(netload, &offset, &netload_sysent, load, NULL);
