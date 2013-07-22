/*
 * NNTPSwitch Accounting for NND JSON Auth Server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include "json-c/json.h"

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"
#include "../curl.h"

#define PREFIX "acct_json: "

PROTO void acct_json(CLIENT *client, CONFIG *cf, char *args)
{
    char tmp[1024];
    struct curlMemStruct curlData;

    if ( (strstr(args, "%s")) == NULL ) {
        syslog(LOG_ERR, PREFIX "No valid url found, check configuration.");
        return;
    }

    // char * escuser = curl_easy_escape(curl, authres->username, 0);
    // http://localhost:82/auth/logout.pl?user=%s&host=%s&pid=%d&b=%lu&a=%u&pb=%lu&pa=%u&p=%s
    sprintf(tmp, args, 
            client->logname, client->hostname, getpid(),
            client->bytes, client->articles, client->postbytes, client->posts,
            client->profile
    );
    // curl_free(escuser);

    syslog(LOG_DEBUG, PREFIX "found url: %s", tmp);

    if ( ! curlGetURL(tmp, &curlData) ) {
        syslog(LOG_ERR, PREFIX "Can't perform curlGetURL().");
        syslog(LOG_ERR, PREFIX "Can't logout user: %s", tmp);
        goto cleanup;
    }

    struct json_object *js_obj, *js_tmp;
    js_obj = json_tokener_parse(curlData.mem);

    /* Message Parsing */
    js_tmp = json_object_object_get(js_obj, "ok");
    if ( ! js_tmp ) {
        syslog(LOG_ERR, PREFIX "Required JSON variable \"ok\" missing from AuthServer.");
        syslog(LOG_ERR, PREFIX "Can't logout user: %s", tmp);
        goto cleanup;
    }
    int ok = json_object_get_int(js_tmp);
    if ( ! ok ) {
        syslog(LOG_ERR, PREFIX "Required JSON variable \"ok\" is not true from AuthServer.");
        syslog(LOG_ERR, PREFIX "Can't logout user: %s", tmp);
        goto cleanup;
    }

    /* Accounting modules do no further processing. */

cleanup:
    if ( curlData.mem ) 
        free(curlData.mem);
    return;
}

