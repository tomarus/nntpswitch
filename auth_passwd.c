#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

#define _MAX_LINEBUF 256
#define _MAX_USERPASS 32
#define _MAX_REASON _MAX_LINEBUF-(2*_MAX_USERPASS)-4

PROTO void auth_passwd(AUTHRESULT *authres, CONFIG *cf, char *args) 
{
	FILE *FILE;
	char retbuf[_MAX_LINEBUF];
	char u[_MAX_USERPASS];
	char p[_MAX_USERPASS];
	char r[_MAX_REASON];

	if ( ! (FILE=fopen(args, "r")) ) 
	{
		syslog(LOG_ERR, "auth_passwd: Error opening password file \"%s\" (%m)", args);
		authres->message = strdup(MSG_AUTH_ERR);
		return;
	}

	while( fgets(retbuf, _MAX_LINEBUF-1, FILE) )
	{
		if ( retbuf[0] == '#' || retbuf[0] == ';' ) {
			continue;
		}

		sscanf(retbuf,"%32[^:]:%32[^:]:%128[^\r\n]", u, p, r);

		if ( ! strcasecmp(u, authres->username) )
		{
			if ( ! strcasecmp(p, authres->password) )
			{
				if ( ! strncasecmp(r, "ok", 2) )
				{
					fclose(FILE);
					authres->authenticated = true;
					authres->message = strdup(MSG_AUTH_OK);
					return;
				} 
				else if ( ! strncasecmp(r, "np", 2) )
				{
					fclose(FILE);
					authres->authenticated = true;
					authres->posting = 0;
					authres->message = strdup(MSG_AUTH_OK);
					return;
				}
				else
				{
					fclose(FILE);
					sprintf(retbuf, "482 %s", r);
					authres->message = strdup(retbuf);
					return;
				}
			} 
			else 
			{
				fclose(FILE);
				authres->message = strdup(MSG_AUTH_REJ);
				return;
			}
		}
	}
	fclose(FILE);

	authres->message = strdup(MSG_AUTH_REJ);
	return;
}

