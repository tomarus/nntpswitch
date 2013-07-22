/* acct_radius.c */

/* libradius.so */
#include <radlib.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

PROTO void acct_radius(CLIENT *client, CONFIG *cfg, char *args)
{
	char hostname[128];
	char server[128];
	char secret[128];
	char sessionid[256];
	struct servent *rport;
	struct rad_handle *rada;
	int ret;
	int port;
	struct in_addr c_ip;
	struct sockaddr_in svraddr;
	socklen_t svraddrlen = sizeof(svraddr);

	if ( sscanf(args, "%128[^:]:%s", server, secret) != 2 )
	{
		syslog(LOG_ERR,"Wrong argument syntax %s for acct_radius", args);
		return;
	}
			    
	if ( !(rada = rad_acct_open()) )
	{
		syslog(LOG_ERR, "acct_radius: rad_acct_open failed");
		return;
	}

	if ((rport = getservbyname("radius-acct", "udp")) != NULL ) 
		port = ntohs(rport->s_port);
	else
		port = 1813;

	/* TODO make config options; 2 = timeout 5 = max_tries */
	if (rad_add_server (rada, server, port, secret, 2, 5) < 0)
	{
		syslog(LOG_ERR, "acct_radius: rad_add_server failed");
		return;
	}

	if (rad_create_request(rada, RAD_ACCOUNTING_REQUEST) < 0) 
	{
		syslog(LOG_ERR, "acct_radius: rad_create_request failed");
		return;
	}

	/* create all the accounting attributes */

	gethostname(hostname, 128);
	c_ip.s_addr = htonl((long) client->ip4addr);
	sprintf(sessionid, "%u:%u", (int)getpid(), client->id);
	getsockname(client->socket, (struct sockaddr *) &svraddr, &svraddrlen);

	/* TODO  on/off controlled by config.
	// Some radius servers require RAD_NAS_PORT(_TYPE) and some forbid it.
	*/
	/* rad_put_int(rada,RAD_NAS_PORT,	client->id + 1 );
	// rad_put_int(rada,RAD_NAS_PORT_TYPE,	RAD_VIRTUAL);
	*/
	rad_put_addr(rada,RAD_NAS_IP_ADDRESS,	svraddr.sin_addr);
	rad_put_string(rada,RAD_NAS_IDENTIFIER,	hostname);
	rad_put_int(rada,RAD_ACCT_STATUS_TYPE,	RAD_STOP);

	/* Some RADIUS servers drop the accounting packet
	// if the RAD_ACCT_AUTHENTIC attribute is not specified.
	// RFC 2139 says:
	// 		The attribute is optional.
	//		Its value could be RAD_AUTH_REMOTE or RADIUS or LOCAL.
	//		Users who are delivered service without being authenticated
	//		SHOULD NOT generate Accounting records.
	*/
	/* rad_put_int(rada, RAD_ACCT_AUTHENTIC,	RAD_AUTH_RADIUS); */
	rad_put_string(rada,RAD_ACCT_SESSION_ID,	sessionid); 
	rad_put_string(rada,RAD_USER_NAME,		client->username);
	rad_put_addr(rada,RAD_FRAMED_IP_ADDRESS,	c_ip);
	rad_put_int(rada,RAD_ACCT_SESSION_TIME,		time(NULL) - client->starttime);
	rad_put_int(rada,RAD_ACCT_INPUT_OCTETS,		client->postbytes);
	rad_put_int(rada,RAD_ACCT_INPUT_PACKETS,	client->posts);
	rad_put_int(rada,RAD_ACCT_OUTPUT_OCTETS,	client->bytes);
	rad_put_int(rada,RAD_ACCT_OUTPUT_PACKETS,	client->articles);

	switch( ret = rad_send_request(rada) ) 
	{
		case RAD_ACCOUNTING_RESPONSE:
			break;
		default:
			syslog(LOG_ERR
				, "acct_radius: accounting vanished"
				", rad_send_request result was %d", ret);
	}
}

