/*
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include "../nntpd.h"
#include "../aconfig.h"
#include "../aprotos.h"
#include "../curl.h"


PROTO bool check_http_accountsharing(char * user, char * host, int maxhost, int maxconn, char * profile, char * resdata)
{
        char                    tmp[1024];
        CURL *                  curl;
        char *                  escuser;
        struct curlMemStruct    curlData;
        bool ok                 = false;

        curlData.mem = NULL;
        curlData.size = 0;

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if ( !curl )
        {
                syslog(LOG_ERR, "check_http_accountsharing: cant initialize curl!");
                return true;
        }

        escuser = curl_easy_escape(curl, user, 0);
        sprintf(tmp, cfg.AsLoginURL, escuser, host, maxhost, maxconn, getpid(), profile);
        curl_free(escuser);

        if ( ! curlGetCurlURL(tmp, &curlData, curl) ) {
            ok=true;
            goto cleanup;
        }

        long rescode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rescode);

        if ( curlData.mem[0] == 'E' ) {
                // Actual accountsharing error, i.e. too many connections.
                strcpy(resdata, curlData.mem+4);
                ok=false;
        } else if ( curlData.mem[0] == 'O' && curlData.mem[1] == 'K' ) {
                // Everything OK
                strcpy(resdata, curlData.mem+3);
                ok=true;
        } else if ( curlData.mem[0] == '<' && curlData.mem[1] == '?' ) {
                // Unhandled result with html/xml data
                char *stripped = replacern(curlData.mem, ' ');
                syslog(LOG_NOTICE, 
                    "check_http_accountsharing: Unhandled XML results from server. Response code %ld response text %s", 
                    rescode, stripped);
                free(stripped);
                strcpy(resdata, curlData.mem);
                ok=true;
        } else {
                // Unparsable/Unhandled result other than OK/ERR/<?xml
                char *stripped = replacern(curlData.mem, ' ');
                syslog(LOG_NOTICE, "check_http_accountsharing: Unparsable results from server. Reponse code %ld reponse text %s", 
                    rescode, stripped);
                free(stripped);
                strcpy(resdata, curlData.mem);
                ok=true;
        }

cleanup:
        curl_easy_cleanup(curl);
        if ( curlData.mem ) 
                free(curlData.mem);
        return ok;
}


PROTO bool http_accountsharing_dropuser(char * user, char * host)
{
        char                    tmp[1024];
        CURL *                  curl;
        char *                  escuser;
        struct curlMemStruct    curlData;
        bool ok                 = false;

        curlData.mem = NULL;
        curlData.size = 0;

        curl_global_init(CURL_GLOBAL_ALL);
        curl = curl_easy_init();
        if ( !curl )
        {
                syslog(LOG_ERR, "http_accountsharing_dropuser: cant initialize curl!");
                return false;
        }

        escuser = curl_easy_escape(curl, user, 0);
        sprintf(tmp, cfg.AsLogoutURL, escuser, host, getpid());
        curl_free(escuser);

        if ( ! curlGetCurlURL(tmp, &curlData, curl) ) {
            syslog(LOG_ERR, "http_accountsharing_dropuser: cant drop user %s", tmp);
            goto cleanup;
        }

        long rescode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &rescode);

        if ( curlData.size == 0 ) {
                // Zero size response.
                syslog(LOG_NOTICE, "http_accountsharing_dropuser: Got zero size response, status %ld", rescode);
        } else if ( curlData.mem[0] == 'E' ) {
                // Real error from AuthServer
                syslog(LOG_NOTICE, "http_accountsharing_dropuser: AuthServer Application Error: %s", curlData.mem);
        } else if ( curlData.mem[0] == 'O' && curlData.mem[1] == 'K' ) {
                // All ok, user dropped
                ok=true;
        } else if ( curlData.mem[0] == '<' && curlData.mem[1] == '?' ) {
                // Unhandled result with html/xml data
                char *stripped = replacern(curlData.mem, ' ');
                syslog(LOG_NOTICE, 
                    "http_accountsharing_dropuser: Unhandled XML results from server. Response code %ld response text %s", 
                    rescode, stripped);
                free(stripped);
        } else {
                // Unparsable/Unhandled result other than OK/ERR/<?xml
                char *stripped = replacern(curlData.mem, ' ');
                syslog(LOG_ERR, 
                    "http_accountsharing_dropuser: Unparsable results from server. Reponse code %ld reponse text %s", 
                    rescode, stripped);
                free(stripped);
        }

        if ( ok == false ) {
                syslog(LOG_ERR, "http_accountsharing_dropuser: cant drop user %s", tmp);
        }

cleanup:
        curl_easy_cleanup(curl);
        if ( curlData.mem ) 
                free(curlData.mem);
        return ok;
}

