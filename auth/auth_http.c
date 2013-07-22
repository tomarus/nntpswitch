/*
 * $Id: auth_http.c,v 1.4 2009-12-01 15:09:33 tommy Exp $
 * (c) 2009 News-Service Europe bv
 *
 * NNTPSwitch Auth for HTTP iov dreamload
 *
 * http://www.dreamload.de/services/userstatus.aspx?user=xx&password=yy
 * Return:
 * authtrue/false:posttruefalse:bytesleft:profile
 * al het andere wat returned wordt is auth rejected
 *
 * uses libcurl (apt-get libcurl3-openssl-dev)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"
#include "../curl.h"

#define PREFIX "auth_http: "

extern	int	   strip;
extern	char	   dbarg[MAX_NAME];

PROTO void auth_http(AUTHRESULT *authres, CONFIG *cf, char *args)
{
	char			tmp[1024];
	unsigned long long	res_bytes;
	int			res_auth;
	int			res_post;
	char			res_profile[MAX_NAME];
	PROFILE *		pf;
	struct curlMemStruct	curlData;

	/*
	 * use %S for stripped username, %s for regular username
	 */
	char *str;
	if ( (str=strstr(args, "%S")) != NULL )
	{
		str[1] = 's';
		sprintf(tmp, args, authres->username_s, authres->password);
		authres->logname = strdup( authres->username_s );
	}
	else
	{
		if ( (str = strstr(args, "%s")) == NULL )
		{
			syslog(LOG_ERR, PREFIX "No valid url found, check configuration");
			authres->message = strdup(MSG_AUTH_ERR);
			return;
		}

		sprintf(tmp, args, authres->username_s, authres->password);
		authres->logname = strdup( authres->username_s );
	}

	// syslog(LOG_DEBUG, PREFIX "found url: %s", tmp);
	if ( ! curlGetURL(tmp, &curlData) ) {
		authres->message = strdup(MSG_AUTH_ERR);
		goto cleanup;
	}

	int n;
        if ( (n=sscanf(curlData.mem, "%d:%d:%lld:%32s", &res_auth, &res_post, &res_bytes, res_profile)) != 4 )
	{
		//syslog(LOG_NOTICE, PREFIX "Unparsable (%d): %s", n, curlData.mem);
		authres->message = strdup(MSG_AUTH_REJ);
		goto cleanup;
	}
	else
	{
		if ( res_auth == 0 )
		{
			authres->message = strdup(MSG_AUTH_REJ);
			goto cleanup;
		}

		if ( (pf=getprofile(res_profile)) == NULL )
		{
			syslog(LOG_ERR, PREFIX "Unknown profile: %s", res_profile);
			authres->message = strdup(MSG_AUTH_ERR);
			goto cleanup;
		}

		authres->authenticated = true;
		authres->bytes = res_bytes;
		authres->posting = res_post;
		authres->profile = strdup(res_profile);
		authres->message = strdup(MSG_AUTH_OK);
		syslog(LOG_NOTICE, PREFIX "login %s profile %s", authres->username, authres->profile);
	}

cleanup:
	if ( curlData.mem ) 
		free(curlData.mem);
	return;
}

