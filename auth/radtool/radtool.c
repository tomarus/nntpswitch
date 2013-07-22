
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <radlib.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "tgetopt.h"


struct option opts[] = {
	{ "help",	0, NULL, 'h', 0 },
	{ "hostname",	1, NULL, 'n', 0 },
	{ "port",	1, NULL, 'q', 0 },
	{ "sessionid",	1, NULL, 's', 0 },
	{ "username",	1, NULL, 'u', 0 },
	{ "clientaddr",	1, NULL, 'c', 0 },
	{ "time",	1, NULL, 't', 0 },
	{ "postbytes",	1, NULL, 'o', 0 },
	{ "posts",	1, NULL, 'p', 0 },
	{ "articles",	1, NULL, 'a', 0 },
	{ "bytes",	1, NULL, 'b', 0 },
	{ "server",	1, NULL, 'r', 0 },
	{ "key",	1, NULL, 'k', 0 },
	{ NULL,		0, NULL, 0,   0 },
};

static const char* usage = "Usage: %s [options]\n"
    "   -n, --hostname NAME \t\tNAS Hostname.\n"
    "   -q, --port PORT     \t\tNAS Port (default from /etc/services).\n"
    "   -s, --sessionid ID  \t\tClient Session ID.\n"
    "   -u, --username NAME \t\tClient Username.\n"
    "   -c, --clientaddr IP \t\tClient Address.\n"
    "   -t, --time SECS     \t\tTotal time connected.\n"
    "   -o, --postbytes BYTES\tPosted Bytes.\n"
    "   -p, --posts NUM     \t\tPosted Articles.\n"
    "   -b, --bytes BYTES   \t\tDownloaded Bytes.\n"
    "   -a, --articles NUM  \t\tDownloaded Articles.\n"
    "   -r, --server NAME   \t\tRemote Server Name/Address.\n"
    "   -k, --key NAME      \t\tRemote Server Shared Key.\n"
    "   -h, --help          \t\tCommand usage.\n";

struct
{
	char *			nashostname;
	int			port;
	char *			sessionid;
	char *			username;
	char *			clientaddr;
	int			seconds;
	unsigned long long	postbytes;
	unsigned int		posts;
	unsigned long long	bytes;
	unsigned int		articles;
	char *			radserver;
	char *			radsecret;
} par;


int acct_entry (void)
{
	struct servent *rport;
	struct rad_handle *rada;
	int port;
	struct in_addr c_ip;

	struct hostent *he = NULL;
	he = gethostbyname( toptargs('n') );
	if ( he == NULL ) 
	{
		fprintf(stderr, "Can't resolve %s", toptargs('n') );
		return 1;
	}

	errno = 0;
	if (!(rada = rad_acct_open())) 
	{
		fprintf (stderr,
			      "rad_acct_open failed (%s)",
			      errno ? strerror (errno) : "?");
		return 1;
	}

	/* set default port */
	if ((rport = getservbyname ("radius-acct", "udp")) != NULL)
		port = ntohs (rport->s_port);
	else
		port = 1813;
	
	/* might be set on cmdline */
	if ( par.port )
		port = par.port;

	/* TODO make config options; 2 = timeout 5 = max_tries */
	if (rad_add_server (rada, toptargs('r'), port, toptargs('k'), 2, 5) < 0) 
	{
		fprintf (stderr,
			      "rad_add_server failed (%s)",
			      rad_strerror (rada));
		return 1;
	}

	if (rad_create_request (rada, RAD_ACCOUNTING_REQUEST) < 0) 
	{
		fprintf (stderr,
			      "rad_create_request failed (%s)",
			      rad_strerror (rada));
		return 1;
	}

	/* create all the accounting attributes */

	c_ip.s_addr = inet_addr( toptargs('c') );

	/* TODO  on/off controlled by config.
	//  Some radius servers require RAD_NAS_PORT(_TYPE)
	//  and some forbid it.
	*/
	/* rad_put_int(rada,RAD_NAS_PORT,               client->id + 1 );
	// rad_put_int(rada,RAD_NAS_PORT_TYPE,       RAD_VIRTUAL);
	*/
	rad_put_addr (rada, RAD_NAS_IP_ADDRESS, * ((struct in_addr *)he->h_addr_list[0]) );
	rad_put_string (rada, RAD_NAS_IDENTIFIER, toptargs('n') );
	rad_put_int (rada, RAD_ACCT_STATUS_TYPE, RAD_STOP);

	/* Some RADIUS servers drop the accounting packet
	// if the RAD_ACCT_AUTHENTIC attribute is not specified.
	// RFC 2139 says:
	//           The attribute is optional.
	//           Its value could be RAD_AUTH_REMOTE or RADIUS or LOCAL.
	//           Users who are delivered service without being authenticated
	//           SHOULD NOT generate Accounting records.
	*/
	/* rad_put_int(rada, RAD_ACCT_AUTHENTIC, RAD_AUTH_RADIUS); */
	rad_put_string (rada, RAD_ACCT_SESSION_ID, toptargs('s') );
	rad_put_string (rada, RAD_USER_NAME, toptargs('u') );
	rad_put_addr (rada, RAD_FRAMED_IP_ADDRESS, c_ip);
	rad_put_int (rada, RAD_ACCT_SESSION_TIME, atoll(toptargs('t')) );
	rad_put_int (rada, RAD_ACCT_INPUT_OCTETS, atoll(toptargs('o')) );
	rad_put_int (rada, RAD_ACCT_INPUT_PACKETS, atoll(toptargs('p')) );
	rad_put_int (rada, RAD_ACCT_OUTPUT_OCTETS, atoll(toptargs('b')) );
	rad_put_int (rada, RAD_ACCT_OUTPUT_PACKETS, atoll(toptargs('a')) );

	if (rad_send_request (rada) != RAD_ACCOUNTING_RESPONSE) 
		return 1;

	return 0;
}


int main(int argc, char **argv) 
{
	tgetopt(argc, argv);
	if ( ! (toptset('n') &&
		toptset('s') &&
		toptset('u') &&
		toptset('c') &&
		toptset('t') &&
		toptset('o') &&
		toptset('p') &&
		toptset('b') &&
		toptset('a') &&
		toptset('r') &&
		toptset('k')
		) || toptset('h') ) { 
			printf(usage, argv[0]);
			return 1;
	}

	par.nashostname = toptargs('n');
	par.port	= atoi( toptargs('q') );
	par.sessionid	= toptargs('s');
	par.username	= toptargs('u');
	par.clientaddr	= toptargs('c');
	par.seconds	= atoi(  toptargs('t') );
	par.postbytes	= atoll( toptargs('o') );
	par.posts	= atoi(  toptargs('p') );
	par.bytes	= atoll( toptargs('b') );
	par.articles	= atoi(  toptargs('a') );
	par.radserver	= toptargs('r');
	par.radsecret	= toptargs('k');

	return acct_entry();
}
