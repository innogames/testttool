#ifndef _CHECK_DNS_H_
#define _CHECK_DNS_H_

#include <vector>
#include <sstream>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/util.h>

#include "healthcheck.h"

/*
   According to RFC1035 4.2.1:
   Messages carried by UDP are restricted to 512 bytes (not counting the IP or UDP headers).
*/
#define DNS_BUFFER_SIZE 512

/* This struct comes from libdns. */
struct dns_header {
		unsigned qid:16;

#if (defined BYTE_ORDER && BYTE_ORDER == BIG_ENDIAN) || (defined __sun && defined _BIG_ENDIAN)
		unsigned qr:1;
		unsigned opcode:4;
		unsigned aa:1;
		unsigned tc:1;
		unsigned rd:1;

		unsigned ra:1;
		unsigned unused:3;
		unsigned rcode:4;
#else
		unsigned rd:1;
		unsigned tc:1;
		unsigned aa:1;
		unsigned opcode:4;
		unsigned qr:1;

		unsigned rcode:4;
		unsigned unused:3;
		unsigned ra:1;
#endif

		unsigned qdcount:16;
		unsigned ancount:16;
		unsigned nscount:16;
		unsigned arcount:16;
}; /* struct dns_header */


class Healthcheck_dns: public Healthcheck {

	/* Methods */
	public:
		Healthcheck_dns(istringstream &definition, class LbNode *_parent_lbnode);

	protected:
		static void callback(evutil_socket_t fd, short what, void *arg);


	/* Members */
	public:
		int schedule_healthcheck();

	private:
		int		 socket_fd;
		struct event	*ev;
		char		*dns_query;

		/* Each check is run with different transaction id. */
		uint16_t	 my_transaction_id;
		static uint16_t	 global_transaction_id;

};

#endif

