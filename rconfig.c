/*
 * NNTPSwitch Remote Configuration CURL/JSON/AuthServer stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include "json-c/json.h"

#include "nntpd.h"
#include "aconfig.h"
#include "aprotos.h"
#include "curl.h"

#define PREFIX "rconfig: "

/* Get all wildmats from AuthServer. */
PROTO bool rconf_getwmats(void)
{
    struct json_object *js_obj, *js_tmp, *js_tmpnam, *js_tmppat;
    struct array_list * js_arr;
    struct curlMemStruct curlData;
    WILDMAT *wm;
    int i;

    syslog(LOG_DEBUG, PREFIX "Preparing rconf_getwildmats()");

    if ( ! curlGetURL(cfg.WildmatURL, &curlData) ) {
        syslog(LOG_ERR, PREFIX "Cant update wildmats from AuthServer.");
        return false;
    }

    js_obj = json_tokener_parse(curlData.mem);
    js_arr = json_object_get_array(js_obj);
    if ( ! js_arr ) {
        syslog(LOG_ERR, PREFIX "Cant update wildmats from AuthServer, unparseable JSON.");
        return false;
    }

    for(i=0; i < json_object_array_length(js_obj); i++) {
        js_tmp    = json_object_array_get_idx(js_obj, i);
        js_tmpnam = json_object_object_get(js_tmp, "name");
        js_tmppat = json_object_object_get(js_tmp, "pat");

        const char * wmname = json_object_get_string(js_tmpnam);
        const char * wmpatt = json_object_get_string(js_tmppat);

        wm = getwildmatptr(wmname);
        if ( wm == NULL ) {
            strcpy( (master->wildmats+i)->name,    wmname );
            strcpy( (master->wildmats+i)->pattern, wmpatt );
            if ( i > MAX_WILDMATS ) 
                die(PREFIX "Too many Wildmat entries (%d) increase MAX_WILDMATS in nntpd.h", i);
        } else {
            strcpy( wm->name,    wmname );
            strcpy( wm->pattern, wmpatt );
        }
    }
    master->numwildmats = i;

    syslog(LOG_DEBUG, PREFIX "Loaded %d wildmats", master->numwildmats);
    return true;
}

