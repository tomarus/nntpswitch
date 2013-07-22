/* $Id: acct_radns.c,v 1.4 2009-01-12 11:36:27 tommy Exp $
 * (c) 2006 News-Service Europe bv
 * NNTPSwitch Syslog Accounting Module
 * with additional Radius accounting for news-service.com specific clients.
 */
/* Link with: libradius.so */

#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <radlib.h>

#include "../aconfig.h"
#include "../nntpd.h"
#include "../aprotos.h"
#include "log_acct.h"

#define NSE_ACCT_MODULE "radns"

PROTO void acct_radns(CLIENT *client, CONFIG *cfg, char *args)
{
	char hostname[128];
	char server[128];
	char secret[128];
	char sessionid[256];
	struct servent *rport;
	struct rad_handle *rada;
	int port;
	struct in_addr c_ip;
	struct sockaddr_in svraddr;
	socklen_t svraddrlen = sizeof (svraddr);

	/* First do the syslog part.
	// So NSE's accounting is properly done even if radius fails
	*/
	NSE_ACCT_LOG (NSE_ACCT_MODULE, client);

	/* lookup port first since it can be overwritten by config */
	if ((rport = getservbyname ("radius-acct", "udp")) != NULL)
		port = ntohs (rport->s_port);
	else
		port = 1813;

	if (sscanf (args, "%128[^:]:%128[^:]:%d", server, secret, &port) != 3)
	{
		if (sscanf (args, "%128[^:]:%s", server, secret) != 2)
		{
			syslog (LOG_ERR, NSE_ACCT_MODULE ": "
			      "syntax error in config file (%s)", args);
			return;
		}
	}

	errno = 0;
	if (!(rada = rad_acct_open ())) {
		syslog (LOG_ERR, NSE_ACCT_MODULE ": "
			      "rad_acct_open failed (%s)",
			      errno ? strerror (errno) : "?");
		return;
	}

	/* TODO make config options; 2 = timeout 5 = max_tries */
	if (rad_add_server (rada, server, port, secret, 2, 5) < 0) {
		syslog (LOG_ERR, NSE_ACCT_MODULE ": "
			      "rad_add_server failed (%s)",
			      rad_strerror (rada));
		return;
	}

	if (rad_create_request (rada, RAD_ACCOUNTING_REQUEST) < 0) {
		syslog (LOG_ERR, NSE_ACCT_MODULE ": "
			      "rad_create_request failed (%s)",
			      rad_strerror (rada));
		return;
	}

	/* create all the accounting attributes */

	gethostname (hostname, 128);
	c_ip.s_addr = htonl ((long) client->ip4addr);
	sprintf (sessionid, "%u:%u", (int) getpid (), client->id);
	getsockname (client->socket, (struct sockaddr *) &svraddr, &svraddrlen);

	/* TODO  on/off controlled by config.
	//  Some radius servers require RAD_NAS_PORT(_TYPE)
	//  and some forbid it.
	*/
	/* rad_put_int(rada,RAD_NAS_PORT,               client->id + 1 );
	// rad_put_int(rada,RAD_NAS_PORT_TYPE,       RAD_VIRTUAL);
	*/
	rad_put_addr (rada, RAD_NAS_IP_ADDRESS, svraddr.sin_addr);
	rad_put_string (rada, RAD_NAS_IDENTIFIER, hostname);
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
	rad_put_string (rada, RAD_ACCT_SESSION_ID, sessionid);
	rad_put_string (rada, RAD_USER_NAME, client->username);
	rad_put_addr (rada, RAD_FRAMED_IP_ADDRESS, c_ip);
	rad_put_int (rada, RAD_ACCT_SESSION_TIME, time (NULL) - client->starttime);
	rad_put_int (rada, RAD_ACCT_INPUT_OCTETS, client->postbytes);
	rad_put_int (rada, RAD_ACCT_INPUT_PACKETS, client->posts);
	rad_put_int (rada, RAD_ACCT_OUTPUT_OCTETS, client->bytes);
	rad_put_int (rada, RAD_ACCT_OUTPUT_PACKETS, client->articles);

	if (rad_send_request (rada) != RAD_ACCOUNTING_RESPONSE) {
		char nas_addr[INET_ADDRSTRLEN];
		char client_addr[INET_ADDRSTRLEN];

		if (! inet_ntop (AF_INET, &svraddr.sin_addr
					, nas_addr, INET_ADDRSTRLEN))
			sprintf (nas_addr, "0.0.0.0");

		if (! inet_ntop (AF_INET, &client->addr.sin_addr
					, client_addr, INET_ADDRSTRLEN))
			sprintf (client_addr, "0.0.0.0");

		syslog (LOG_ERR, NSE_ACCT_MODULE ": "
				"radius acct server not responding (%s)"
				" [%s %s %s %s \"%s\" %s %ju %llu %u %llu %u]",
				rad_strerror (rada),
				client->profile->Name,
				nas_addr,
				hostname,
				sessionid,
				client->username,
				client_addr,
				(uintmax_t)(time (NULL) - client->starttime),
				client->postbytes,
				client->posts,
				client->bytes,
				client->articles);
	}
}
