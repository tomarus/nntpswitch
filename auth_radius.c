/*
 * radius authorization for nntpswitch
 * original by ads of voicenet. big thanks!
 */

#include <radlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

#define RAD_TIMEOUT 5
#define RAD_RETRIES 5

PROTO void auth_radius(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	struct rad_handle *radh;
	struct servent *rport;
	/* ADS - 20031222 - addition to see if client can post */
	size_t len;
	const void *data;
	char *rmessage = NULL;
	char *reply_msg = NULL;
	/* -- end -- */
	int port;
	char hostname[128];
	char server[128];
	char secret[128];
	char strip[8];
	struct in_addr c_ip;

	/* initialize port nr first, could be overwritten by config */
	if ( (rport = getservbyname("radius","udp")) != NULL ) 
		port = ntohs(rport->s_port);
	else
		port = 1812;

	/* v3 syntax: servername:secret:strip:port */
	if ( sscanf(args, "%128[^:]:%128[^:]:%128[^:]:%d", server, secret, strip, &port) != 4 )
	{
		/* v2 syntax: servername:secret:strip */
		if ( sscanf(args, "%128[^:]:%128[^:]:%s", server, secret, strip) != 3 )
		{
			/* v1 syntax: servername:secret */
			if ( sscanf(args, "%128[^:]:%s", server, secret) == 2 ) 
			{
				strcpy(strip, "strip");
			} else {
				/* config error */
				syslog(LOG_ERR, "auth_radius: Wrong argument syntax \"%s\"", args);
				authres->message = strdup(MSG_AUTH_ERR);
				return;
			}
		}
	}

	/* check for "strip" or "nostrip" syntax */
	if ( ! strcasecmp(strip, "strip") && ! strcasecmp(strip, "nostrip") )
	{
		syslog(LOG_ERR, "auth_radius: config error, parameter should be \"strip\" or \"nostrip\"");
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}


	if ( !(radh = rad_auth_open()) ) 
	{
		syslog(LOG_ERR, "auth_radius: rad_acct_open failed");
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}


	/* TODO make config options; 2 = timeout 5 = max_tries */
	if (rad_add_server (radh, server, port, secret, RAD_TIMEOUT, RAD_RETRIES) < 0)
	{
		syslog(LOG_ERR, "auth_radius: rad_add_server failed");
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if (rad_create_request(radh, RAD_ACCESS_REQUEST) < 0)
	{
		syslog(LOG_ERR, "auth_radius: rad_create_request failed");
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	gethostname(hostname, 128);

	rad_put_int(radh, RAD_NAS_PORT, authres->port);
	rad_put_int(radh, RAD_SERVICE_TYPE, RAD_LOGIN); 

	if ( ! strcasecmp(strip, "strip") )
		rad_put_string(radh, RAD_USER_NAME, authres->username_s);
	else
		rad_put_string(radh, RAD_USER_NAME, authres->username);

	rad_put_string(radh, RAD_USER_PASSWORD, authres->password);
	rad_put_addr(radh, RAD_NAS_IP_ADDRESS, authres->in_addr);
	rad_put_string(radh, RAD_NAS_IDENTIFIER, hostname);

	c_ip.s_addr = htonl((long) client->ip4addr);
	rad_put_addr(radh,RAD_FRAMED_IP_ADDRESS, c_ip);

	if ( (rad_send_request(radh)) == RAD_ACCESS_ACCEPT )
	{
		authres->authenticated = true;
		authres->message = strdup(MSG_AUTH_OK);
	}

	int r;
	int maxattr = 0;
	while ( (r = rad_get_attr(radh, &data, &len)) > 0 && maxattr++ < 50 ) 
	{
		switch ( r )
		{
			case RAD_FILTER_ID:
				rmessage = rad_cvt_string(data, len);
				if ( !strncmp(rmessage, "nopost", 6) ) 
					authres->posting = false;
				if ( rmessage != NULL) 
					free(rmessage);
				break;

			case 78:
				/* 78 == Configuration-Token (profiel) */
				rmessage = rad_cvt_string(data, len);
				authres->profile = strdup(rmessage);
				if ( rmessage != NULL) 
					free(rmessage);
				break;

			case RAD_REPLY_MESSAGE:
				reply_msg = rad_cvt_string(data, len);
				break;

			case RAD_VENDOR_SPECIFIC:
				/* TODO: should be implemented */
				// syslog(LOG_DEBUG, "auth_radius: got vendor-specific attribute with size %d", len);
				break;

			default:
				syslog(LOG_DEBUG, "auth_radius: unknown attribute: %d", r);
		}
	}

	rad_close(radh);

	char buf[1024];
	if ( authres->authenticated != true ) 
	{
		if ( reply_msg != NULL ) {
			snprintf(buf, 1023, "482 %s", reply_msg);
			authres->message = strdup(buf);
			free(reply_msg);
		} else 
			authres->message = strdup(MSG_AUTH_REJ);

		syslog(LOG_DEBUG, "auth_radius: Denied Login for %s (%s)"
					, authres->username, authres->message);

	} else {
		if ( reply_msg != NULL ) {
			snprintf(buf, 1023, "281 %s", reply_msg);
			authres->message = strdup(buf);
			free(reply_msg);
		} else 
			authres->message = strdup(MSG_AUTH_OK);

		syslog(LOG_DEBUG, "auth_radius: Successful Login for %s (%s)"
					, authres->username, authres->message);
	}

}
