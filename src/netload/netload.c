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

#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>
#include <unistd.h>

#include "netload.h"

#define FACTOR 128.0

static void usage (void);

static void
usage (void)
{
	fprintf (stderr, "netloadt [-v] [-l timeout] [-w] [-s -d device -u unit]\n");
	exit (1);
}

static char std_form[]=
		"\f"
		"Interface: %s%d\n"
		"\n"
		"Samples for average: %d\n"
		"Current (Kbit/s): In: %f Out: %f\n"
  		"Total packets: In: %lu Out: %lu\n"
  		"Total bytes: In: %lu Out: %lu\n"
		"Average (Kbit/s): In %f Out: %f\n"
		"1 Kbit/s = %f bytes/s\n";

static char html_form[]=
		"Refresh: 1s\n"
		"Content-Type: text/html\n"
		"\n"
		"<html>"
		"<head>"
		"<title>Traffic Counters for Interface: %s%d</title>"
		"</head>"
		"<body>"
		"Samples for average: %d<br>"
		"Current (Kbit/s): In: %f Out: %f<br>"
  		"Total packets: In: %lu Out: %lu<br>"
  		"Total bytes: In: %lu Out: %lu<br>"
		"Average (Kbit/s): In %f Out: %f<br>"
		"1 Kbit/s = %f bytes/s<br>"
		"</body></html>";

static char * cur_form = html_form;

int
main(int argc, char **argv)
{
static struct _if_data ifdt, *id = &ifdt;
float in, out;
int i;
char *endptr;
int syscall_num, verbose=0, loop=0, set=0, html=1;
struct module_stat stat;

	while ((i = getopt(argc, argv, "d:u:l:svwh")) != -1)
		switch (i) {
		case 'l':
			loop=atoi(optarg);
			break;
		case 'd':
			strcpy(id->if_name, optarg);
			break;
		case 'u':
			id->if_unit=atoi(optarg);
			break;
		case 's':
			set++;
			break;
		case 'v':
			verbose++;
			break;
		case 'w':
			html--;
			cur_form=std_form;
			break;
		case 'h':
		default:
			usage();
		}
/*
	argc -= optind;
	argv += optind
*/
	stat.version = sizeof(stat);
	modstat(modfind("netload"), &stat);
	syscall_num = stat.data.intval;
	if (verbose)
	  printf("%d\n", syscall_num);
	if (set) 
	  return syscall (syscall_num, LOAD_SET, 0, id);
	do {
		syscall (syscall_num, LOAD_READ, 0, id);

		in=out=0;
		for (i=0; i<MAX_STAT; i++) {
		  in+=id->stat[i].in/FACTOR;
		  out+=id->stat[i].out/FACTOR;
		};

  		printf(cur_form,
 id->if_name, id->if_unit, MAX_STAT,
 id->stat[id->stat_counter].in/FACTOR, id->stat[id->stat_counter].out/FACTOR,
 id->if_packets.out, id->if_packets.out,
 id->if_total.in, id->if_total.out,
 in/MAX_STAT, out/MAX_STAT,
 FACTOR
		);
		if (loop)
		  sleep(loop);
	} while(loop);
	return 0;
}
