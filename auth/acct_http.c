/*
 * $Id: acct_http.c,v 1.3 2009-10-07 12:51:32 tommy Exp $
 * (c) 2009 News-Service Europe bv
 * NNTPSwitch HTTP Accounting
 */

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"
#include "../log.h"
#include "log_acct.h"
#include <curl/curl.h>

#define NSE_ACCT_MODULE "acct_http"
#define PREFIX NSE_ACCT_MODULE ": "


// Null Callback coz we're not interrested in returned body
static size_t curlNullWriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
        size_t realsize = size * nmemb;
	return realsize;
}

PROTO void acct_http(CLIENT *client, CONFIG *cfg, char *args)
{
	char			tmp[1024];
	CURL *			curl;
	CURLcode		res;
	char			errormsg[CURL_ERROR_SIZE];
	char *			nullPtr;

	// do nse acct first
	NSE_ACCT_LOG (NSE_ACCT_MODULE, client);

	// create url from config, add parameters
	sprintf(tmp, "%s?user=%s&bytes=%lld&articles=%d&postbytes=%lld&posts=%d",
			args, client->logname ? client->logname : client->username,
			client->bytes, client->articles, client->postbytes, client->posts);

	syslog(LOG_DEBUG, PREFIX "Accounting url: %s", tmp);

	// Start CURL from here


	curl_global_init(CURL_GLOBAL_ALL);

	curl = curl_easy_init();
	if ( !curl )
	{
		syslog(LOG_ERR, PREFIX "cant initialize curl!");
		return;
	}

#ifdef SKIP_PEER_VERIFICATION
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

	curl_easy_setopt(curl, CURLOPT_URL, tmp);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errormsg);
	// curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "nntpswitch-libcurl/1.0");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlNullWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&nullPtr);

	res = curl_easy_perform(curl);

	if ( res )
	{
		syslog(LOG_ERR, PREFIX "cant perform curl!");
		goto cleanup;
	}

	// syslog(LOG_DEBUG, PREFIX "Accounting done");

cleanup:
	curl_easy_cleanup(curl);
	return;
}

