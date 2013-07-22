#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "aconfig.h"
#include "nntpd.h"
#include "aprotos.h"

#define _MAX_LINEBUF 256

PROTO char* handshake_pop3(int sock, char *buf, int size, int timeout) 
{
	int n;

	if ( (n=read_socket(sock, buf, size, timeout)) <= 0 )
		return("Connection Failed: %m");

	buf[n] = 0;

	if ( buf[0] != '+' && buf[1] != 'O' && buf[2] != 'K' ) 
		return buf;

	return NULL;
}


PROTO void auth_pop3(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	char tmp[_MAX_LINEBUF];
	char server[128];
	int port;
	int sock;
	char *err;

	if ( sscanf(args, "%128[^:]:%d", server, &port) != 2 ) 
	{
		syslog(LOG_ERR, "auth_pop3: Wrong argument syntax \"%s\"", args);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( (sock=connect_socket(server, port)) == -1 ) 
	{
		syslog(LOG_ERR, "auth_pop3: Could not connect to POP3 server %s", server);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( (err=handshake_pop3(sock, tmp, _MAX_LINEBUF, cf->ServerReadTimeout)) != NULL )
	{
		syslog(LOG_ERR, "auth_pop3: Server %s failed: %s", server, err);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	sprintf(tmp, "USER %s\r\n", authres->username_s);
	write_socket(sock, tmp, strlen(tmp), cf->ServerReadTimeout);
	if ( read_socket(sock, tmp, _MAX_LINEBUF, cf->ServerReadTimeout) <= 0 )
	{
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	sprintf(tmp, "PASS %s\r\n", authres->password);
	write_socket(sock, tmp, strlen(tmp), cf->ServerReadTimeout);
	if ( read_socket(sock, tmp, _MAX_LINEBUF, cf->ServerReadTimeout) <= 0 )
	{
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	if ( strncasecmp("+OK", tmp, 3) == 0 ) 
	{
		sprintf(tmp, "QUIT\r\n");
		write_socket(sock, tmp, strlen(tmp), cf->ServerReadTimeout);
		close(sock);

		authres->authenticated = true;
		authres->message = strdup(MSG_AUTH_OK);
		return;
	}
	authres->message = strdup(MSG_AUTH_REJ);
	return;
}

