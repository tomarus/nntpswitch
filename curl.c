/*
 * NNTPSwitch Curl Helper Functions
 */

#include <stdlib.h>
#include <string.h>
#include <sys/syslog.h>
#include <curl/curl.h>
#include "curl.h"

static void * curlRealloc(void *ptr, size_t size)
{
    if (ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}

size_t curlWriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    struct curlMemStruct *mem = (struct curlMemStruct *)data;

    mem->mem = curlRealloc(mem->mem, mem->size + realsize + 1);
    if (mem->mem) {
        memcpy( &(mem->mem[mem->size]), ptr, realsize );
        mem->size += realsize;
        mem->mem[mem->size] = 0;
    }
    return realsize;
}

char * curlGetURL(const char *url, struct curlMemStruct *curlData)
{
    CURL * curl;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if ( !curl ) {
        syslog(LOG_ERR, "curlGetURL: cant initialize curl!");
        return NULL;
    }
    char * memptr = curlGetCurlURL(url, curlData, curl);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return memptr;
}

char * curlGetCurlURL(const char *url, struct curlMemStruct *curlData, CURL * curl)
{
    char     errormsg[CURL_ERROR_SIZE];
    CURLcode res;

    curlData->mem  = NULL;
    curlData->size = 0;

#ifdef SKIP_PEER_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif
#ifdef SKIP_HOSTNAME_VERIFICATION
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errormsg);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "nntpswitch-libcurl/1.0");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)curlData);

    res = curl_easy_perform(curl);
    if ( res ) {
        syslog(LOG_ERR, "curlGetCurlURL: cant perform curl: %s", errormsg);
        return NULL;
    }
    if ( ! curlData->mem ) {
        syslog(LOG_ERR, "curlGetCurlURL: cant perform curl empty response");
        return NULL;
    }

    return curlData->mem;
}

char * curlPostCurlURL(const char *url, struct curlMemStruct *curlData, CURL * curl, const char * postdata)
{
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postdata);
    return curlGetCurlURL(url, curlData, curl);
}

