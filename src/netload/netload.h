#ifndef NETLOAD_H
#define NETLOAD_H

#define MAX_IFACES 1

/* Should be a power of 2 +1, < MAXINT */
#define MAX_STAT 4

struct _if_data {
	char if_name[16];
	short if_unit;

	struct {
	  u_long in;
	  u_long out;
	  }	if_total,
		if_packets,
		stat[MAX_STAT];
	int stat_counter;
	};

#define LOAD_READ 0x00000001
#define LOAD_SET  0x00000002

#endif /* NETLOAD_H */
