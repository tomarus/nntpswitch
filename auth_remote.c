#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

#define _MAX_LINEBUF 256

int connect_to_server (char *server, int port, int timeout)
{
	int sock;

	if ((sock = connect_socket (server, port)) == -1)
		return -1;
 
	char response[cfg.BufSize];
	if (handshake_nntp (sock, response, timeout) != NULL) {
		close (sock);
		return -1;
	}

	return sock;
}

PROTO void auth_remote(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	char tmp[_MAX_LINEBUF];
	char server[128];
	int port;
	int sock;

	if ( sscanf(args, "%128[^:]:%d", server, &port) != 2 ) 
	{
		syslog(LOG_ERR, "auth_remote: Wrong argument syntax \"%s\"", args);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( (sock=connect_to_server(server, port, cf->ServerReadTimeout)) == -1 ) 
	{
		syslog(LOG_ERR, "auth_remote: Error connecting to authentication server"); 
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	sprintf(tmp, "AUTHINFO USER %s\r\n", authres->username_s);
	write_socket(sock, tmp, strlen(tmp), cf->ServerReadTimeout);
	if ( read_socket(sock, tmp, _MAX_LINEBUF, cf->ServerReadTimeout) <= 0 )
	{
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	sprintf(tmp, "AUTHINFO PASS %s\r\n", authres->password);
	write_socket(sock, tmp, strlen(tmp), cf->ServerReadTimeout);
	if ( read_socket(sock, tmp, _MAX_LINEBUF, cf->ServerReadTimeout) <= 0 )
	{
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( atoi(tmp) == 281 ) 
	{
		authres->authenticated = true;
		authres->message = strdup(MSG_AUTH_OK);
		return;
	}

	authres->message = strdup(MSG_AUTH_REJ);
	return;
}

