#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"

PROTO void auth_ignore(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	if ( strcasecmp(args, "true") == 0 )
	{
		authres->authenticated = true;
		authres->message = strdup(MSG_AUTH_OK);
		return;
	}
	authres->message = strdup(MSG_AUTH_REJ);
	return;
}

