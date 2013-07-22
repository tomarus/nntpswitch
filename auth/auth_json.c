/*
 * NNTPSwitch Auth for NND JSON Auth Server
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

#define PREFIX "auth_json: "

PROTO void auth_json(AUTHRESULT *authres, CONFIG *cf, char *args)
{
    char tmp[1024];
    struct curlMemStruct curlData;

    if ( (strstr(args, "%s")) == NULL ) {
        syslog(LOG_ERR, PREFIX "No valid url found, check configuration");
        authres->message = strdup(MSG_AUTH_ERR);
        return;
    }
    authres->logname = strdup( authres->username );

    // char * escuser = curl_easy_escape(curl, authres->username, 0);
    // char * escpass = curl_easy_escape(curl, authres->password, 0);
    // http://localhost:82/auth/login.pl?user=%s&pass=%s&host=%s&pid=%d
    sprintf(tmp, args, authres->logname, authres->password, authres->hostname, getpid());
    // curl_free(escuser);
    // curl_free(escpass);

    syslog(LOG_DEBUG, PREFIX "found url: %s", tmp);

    if ( ! curlGetURL(tmp, &curlData) ) {
        authres->message = strdup(MSG_AUTH_ERR);
        goto cleanup;
    }

    struct json_object *js_obj, *js_tmp;
    js_obj = json_tokener_parse(curlData.mem);

    /* 
       FIXME Check all possible values from AuthServer.
       Authserver can return full or short versions e.g.:
       For read access AuthServer can return "r" or "read"
       Read Pattern is returned like "rp" or "readpat"
       We should really handle everything here and 
       update the authres struct accordingly.
    */

    /* READ access */
    js_tmp = json_object_object_get(js_obj, "r");
    authres->authenticated = json_object_get_int(js_tmp);

    /* POST access */
    js_tmp = json_object_object_get(js_obj, "p");
    authres->posting = json_object_get_int(js_tmp);

    /* Post Variables */
    js_tmp = json_object_object_get(js_obj, "host");
    if ( js_tmp )
        authres->posthost = strdup(json_object_get_string(js_tmp));
    js_tmp = json_object_object_get(js_obj, "abuse");
    if ( js_tmp )
        authres->postabuse = strdup(json_object_get_string(js_tmp));
    js_tmp = json_object_object_get(js_obj, "org");
    if ( js_tmp )
        authres->postorg = strdup(json_object_get_string(js_tmp));

    /* Disabled */
    js_tmp = json_object_object_get(js_obj, "d");
    authres->disabled = json_object_get_int(js_tmp);
    /* OverQuota */
    js_tmp = json_object_object_get(js_obj, "oq");
    authres->overquota = json_object_get_int(js_tmp);
    if ( authres->disabled || authres->overquota ) {
        authres->authenticated = 0;
    }

    /* Message Parsing */
    js_tmp = json_object_object_get(js_obj, "msg");
    if ( ! js_tmp ) {
        authres->authenticated = false;
        syslog(LOG_ERR, "Required JSON variable \"msg\" missing from AuthServer.");
        goto cleanup;
    }
    if ( authres->authenticated ) 
        sprintf(tmp, "281 %s", json_object_get_string(js_tmp));
    else
        sprintf(tmp, "482 %s", json_object_get_string(js_tmp));
    authres->message = strdup(tmp);

    /* Profile */
    js_tmp = json_object_object_get(js_obj, "prf");
    if ( ! js_tmp ) {
        authres->authenticated = false;
        syslog(LOG_ERR, "Required JSON variable \"prf\" missing from AuthServer.");
        goto cleanup;
    }
    authres->profile = strdup(json_object_get_string(js_tmp));

    /* ReadPat */
    js_tmp = json_object_object_get(js_obj, "rp");
    if ( js_tmp ) authres->readpat = strdup(json_object_get_string(js_tmp));
    /* PostPat */
    js_tmp = json_object_object_get(js_obj, "pp");
    if ( js_tmp ) authres->postpat = strdup(json_object_get_string(js_tmp));

    /* Bytes Left */
    js_tmp = json_object_object_get(js_obj, "b");
    if ( js_tmp ) authres->bytes = json_object_get_int(js_tmp);

    /* Time Left */
    js_tmp = json_object_object_get(js_obj, "t");
    if ( js_tmp ) authres->timeleft = json_object_get_int(js_tmp);

    /* Kbit / Speed */
    js_tmp = json_object_object_get(js_obj, "kb");
    if ( js_tmp ) authres->userkbit = json_object_get_int(js_tmp);

    syslog(LOG_NOTICE, PREFIX "login %s profile %s", authres->username, authres->profile);

cleanup:
    if ( curlData.mem ) 
        free(curlData.mem);
    return;
}

