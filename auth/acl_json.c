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

#define PREFIX "acl_json: "

PROTO void freehttpacl(ACL * acl)
{
    if ( acl != NULL )
        free(acl);
}


PROTO ACL * gethttpacl(char * ipaddr, CURL * curl)
{
    char tmp[1024];
    struct curlMemStruct curlData;

    ACL * acl;
    acl = calloc(1, sizeof(ACL));
    if ( acl == NULL ) {
        syslog(LOG_ERR, "Cant malloc ACL, exiting.");
        return NULL;
    }

    // http://localhost:82/auth/acl.pl?ip=%s
    // {"prf":"default","p":1,"a":1,"r":1,"un":0,"d":0,"s":0,"ap":0}
    sprintf(tmp, cfg.ACLURL, ipaddr);
    syslog(LOG_DEBUG, PREFIX "found url: %s", tmp);

    if ( ! curlGetCurlURL(tmp, &curlData, curl) ) {
        freehttpacl(acl);
        acl = NULL;
        goto cleanup;
    }

    if ( curlData.mem[0] != '{' ) {
        freehttpacl(acl);
        acl = NULL;
        char *stripped = replacern(curlData.mem, ' ');
        syslog(LOG_ERR, "Curl acl didn't return JSON data. Data was: %s", stripped);
        free(stripped);
        goto cleanup;
    }

    static struct json_object *js_obj = NULL, *js_tmp = NULL;
    js_obj = json_tokener_parse(curlData.mem);

    /* Profile Name */
    js_tmp = json_object_object_get(js_obj, "prf");
    acl->profile = getprofile(json_object_get_string(js_tmp));
    if ( acl->profile == NULL ) {
        syslog(LOG_ERR, "Cant get profile from HTTP ACL.");
        goto cleanup;
    }

    /* Flags */
    js_tmp = json_object_object_get(js_obj, "r");
    acl->read = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "p");
    acl->post = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "ap");
    acl->apost = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "d");
    acl->deny = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "a");
    acl->auth = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "s");
    acl->stats = json_object_get_int(js_tmp);
    js_tmp = json_object_object_get(js_obj, "un");
    acl->unlimit = json_object_get_int(js_tmp);

cleanup:
    if ( curlData.mem ) 
        free(curlData.mem);
    if ( js_obj != NULL )
        json_object_put(js_obj);
    return acl;
}

